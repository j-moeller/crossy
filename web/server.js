import express from "express";
import sqlite3 from "sqlite3";
import ejs from "ejs";
import fs from "fs";

const app = express();
const port = 3000;

if (process.argv.length != 3) {
  console.error("usage: node server.js <sqlite>");
  process.exit(1);
}

const dbfile = process.argv[2];
const db = new sqlite3.Database(dbfile);

/**
 * index.ejs
 */
app.get('/', async (req, res) => {
  const reasons = await new Promise((resolve) => {
    db.serialize(() => {
      db.all("SELECT id, reason FROM reason_names", (err, rows) => {
        if (err) {
          reject(err);
        }

        resolve(rows);
      });
    });
  });

  reasons.sort((a, b) => a.reason.localeCompare(b.reason));

  const html = await new Promise((resolve, reject) => {
    ejs.renderFile("web/templates/index.ejs", { reasons: reasons }, {}, function (err, str) {
      if (err) {
        reject();
      }

      resolve(str);
    });
  });

  res.send(html);
});

/**
 * matrix.ejs
 */
app.get('/reason/:id([0-9]+)', async (req, res) => {
  const reason_name_id = req.params.id;
  const reason_name = await fetchReasonName(reason_name_id);

  const [id_list, target_list, inv_target_list, targets] = await fetchTargets();
  const n_targets = target_list.length;

  const aggr = await new Promise((resolve, reject) => {
    db.serialize(() => {
      const sql_request = `SELECT COUNT(*) AS cnt, data.target_1_id, data.target_2_id FROM data
        INNER JOIN reasons ON data.reason_id = reasons.id
        INNER JOIN reason_names ON reason_names.id = reasons.reason_name_id
        WHERE reason_names.id = ${reason_name_id} GROUP BY data.target_1_id, data.target_2_id`;

      db.all(sql_request, (err, rows) => {
        if (err) {
          reject(err);
        }
        resolve(rows);
      });
    });
  });

  const M = Array(n_targets).fill().map(() => Array(n_targets).fill(0));

  aggr.forEach((row) => {
    const i = inv_target_list[targets[row["target_1_id"]]];
    const j = inv_target_list[targets[row["target_2_id"]]];
    const v = row["cnt"];

    M[i][j] += v;
    M[j][i] += v;
  });

  const html = await new Promise((resolve, reject) => {
    const data = {
      reason_name_id: reason_name_id,
      header: reason_name,
      id_list: id_list,
      target_list: target_list.map((t) => printableTarget(t)),
      n_targets: n_targets,
      M: M
    };

    ejs.renderFile("web/templates/matrix.ejs", data, {}, (err, str) => {
      if (err) {
        reject(err);
      }

      resolve(str);
    });
  });

  res.send(html);
});

app.get('/overview', async (req, res) => {
  const [id_list, target_list, inv_target_list, targets] = await fetchTargets();
  const n_targets = target_list.length;

  const aggr = await new Promise((resolve, reject) => {
    db.serialize(() => {
      const sql_request = `SELECT COUNT(*), data.target_1_id, data.target_2_id FROM data
        INNER JOIN reasons ON data.reason_id = reasons.id
        INNER JOIN reason_names ON reason_names.id = reasons.reason_name_id
        GROUP BY data.target_1_id, data.target_2_id`;

      db.all(sql_request, (err, rows) => {
        console.log(rows);
        if (err) {
          reject(err);
        }
        resolve(rows);
      });
    });
  });

  const M = Array(n_targets).fill().map(() => Array(n_targets).fill(new Set()));

  aggr.forEach((row) => {
    const i = inv_target_list[targets[row["target_1_id"]]];
    const j = inv_target_list[targets[row["target_2_id"]]];

    const reason = row["reason"];

    M[i][j].add(reason);
    M[j][i].add(reason);
  });

  for (let i = 0; i < n_targets; i++) {
    for (let j = 0; j < n_targets; j++) {
      M[i][j] = Array.from(M[i][j]);
    }
  }

  const html = await new Promise((resolve, reject) => {
    const data = {
      reason_name_id: undefined,
      header: "Overview",
      id_list: id_list,
      target_list: target_list.map((t) => printableTarget(t)),
      n_targets: n_targets,
      M: M
    };

    ejs.renderFile("web/templates/matrix.ejs", data, {}, (err, str) => {
      if (err) {
        reject(err);
      }

      resolve(str);
    });
  });

  res.send(html);
});

app.get('/overview-cnt', async (req, res) => {
  const [id_list, target_list, inv_target_list, targets] = await fetchTargets();
  const n_targets = target_list.length;

  const aggr = await new Promise((resolve, reject) => {
    db.serialize(() => {
      const sql_request = `SELECT COUNT(*) AS cnt, data.target_1_id, data.target_2_id FROM data
        INNER JOIN reasons ON data.reason_id = reasons.id
        INNER JOIN reason_names ON reason_names.id = reasons.reason_name_id
        GROUP BY data.target_1_id, data.target_2_id`;

      db.all(sql_request, (err, rows) => {
        if (err) {
          reject(err);
        }
        resolve(rows);
      });
    });
  });

  const M = Array(n_targets).fill().map(() => Array(n_targets).fill(0));

  aggr.forEach((row) => {
    const i = inv_target_list[targets[row["target_1_id"]]];
    const j = inv_target_list[targets[row["target_2_id"]]];
    const v = row["cnt"];

    M[i][j] += v;
    M[j][i] += v;
  });

  const html = await new Promise((resolve, reject) => {
    const data = {
      reason_name_id: undefined,
      header: "Overview",
      id_list: id_list,
      target_list: target_list.map((t) => printableTarget(t)),
      n_targets: n_targets,
      M: M
    };

    ejs.renderFile("web/templates/matrix-cnt.ejs", data, {}, (err, str) => {
      if (err) {
        reject(err);
      }

      resolve(str);
    });
  });

  res.send(html);
});

/**
 * inputs.ejs
 */
app.get('/inputs/:target_1_id([0-9]+)/:target_2_id([0-9]+)/:reason_name_id([0-9]+)', async (req, res) => {
  const target_1_id = req.params.target_1_id;
  const target_2_id = req.params.target_2_id;
  const reason_name_id = req.params.reason_name_id;

  const target_1 = await fetchTarget(target_1_id);
  const target_2 = await fetchTarget(target_2_id);

  const reason_name = await new Promise((resolve, reject) => {
    db.serialize(() => {
      db.get(`SELECT reason FROM reason_names WHERE id = ${reason_name_id}`, (err, row) => {
        if (err) {
          reject(err);
        }
        resolve(row.reason);
      });
    });
  });

  const inputs = await new Promise((resolve, reject) => {
    db.serialize(() => {
      db.all(`SELECT inputs.id, inputs.filename FROM data
        JOIN inputs ON data.input_id = inputs.id
        JOIN reasons ON data.reason_id = reasons.id
        WHERE reasons.reason_name_id = ${reason_name_id} AND
        (
          (data.target_1_id = ${target_1_id} AND data.target_2_id = ${target_2_id}) OR
          (data.target_1_id = ${target_2_id} AND data.target_2_id = ${target_1_id})
        )`, (err, rows) => {
        if (err) {
          reject(err);
        }
        resolve(rows);
      });
    });
  });

  const html = await new Promise((resolve, reject) => {
    const data = {
      inputs: inputs,
      target_1_id: target_1_id,
      target_1: target_1,
      target_2_id: target_2_id,
      target_2: target_2,
      reason_name: reason_name
    };

    ejs.renderFile("web/templates/inputs.ejs", data, {}, (err, str) => {
      if (err) {
        reject(err);
      }

      resolve(str);
    });
  });

  res.send(html);
});

/**
 * outputs_detail.ejs
 */
app.get('/outputs/:input_id([0-9]+)/:target_1_id([0-9]+)/:target_2_id([0-9]+)/', async (req, res) => {
  const target_1_id = req.params.target_1_id;
  const target_2_id = req.params.target_2_id;
  const input_id = req.params.input_id;

  const target_1 = await fetchTarget(target_1_id);
  const target_2 = await fetchTarget(target_2_id);

  const input_filename = await new Promise((resolve, reject) => {
    db.serialize(() => {
      db.get(`SELECT filename FROM inputs WHERE id = ${input_id}`, (err, row) => {
        if (err) {
          reject(err);
        }
        resolve(row.filename);
      });
    });
  });

  const input = await readInFile(input_filename);
  const [input_hash, hashes, outputs] = await parseOutFile(input_filename);

  const html = await new Promise((resolve, reject) => {
    const data = {
      input_filename: input_filename,
      input: input,
      input_hash: input_hash,
      target_1: target_1,
      target_2: target_2,
      hashes: hashes,
      outputs: outputs
    };

    ejs.renderFile("web/templates/outputs_detail.ejs", data, {}, (err, str) => {
      if (err) {
        reject(err);
      }

      resolve(str);
    });
  });

  res.send(html);
});

/**
 * outputs.ejs
 */
app.get('/outputs/:input_id([0-9]+)', async (req, res) => {
  const input_id = req.params.input_id;

  const input_filename = await new Promise((resolve, reject) => {
    db.serialize(() => {
      db.get(`SELECT filename FROM inputs WHERE id = ${input_id}`, (err, row) => {
        if (err) {
          reject(err);
        }
        resolve(row.filename);
      });
    });
  });

  const input = await readInFile(input_filename);
  const [input_hash, hashes, outputs] = await parseOutFile(input_filename);

  const html = await new Promise((resolve, reject) => {
    const data = {
      input_filename: input_filename,
      input: input,
      input_hash: input_hash,
      hashes: hashes,
      outputs: outputs
    };

    ejs.renderFile("web/templates/outputs.ejs", data, {}, (err, str) => {
      if (err) {
        reject(err);
      }

      resolve(str);
    });
  });

  res.send(html);
});

app.listen(port, () => {
  console.log(`Example app listening on port ${port}`)
});

function fetchReasonName(reason_name_id) {
  return new Promise((resolve, reject) => {
    db.serialize(() => {
      db.get(`SELECT reason FROM reason_names
      WHERE reason_names.id = ${reason_name_id}`, (err, row) => {
        if (err) {
          reject(err);
        }
        resolve(row.reason);
      });
    });
  });
}

async function fetchTargets() {
  const indexOf = await new Promise((resolve, reject) => {
    fs.readFile("web/order.json", { encoding: "utf-8" }, (err, data) => {
      if (err) {
        reject(err);
      }

      resolve(JSON.parse(data));
    });
  });

  return new Promise((resolve, reject) => {
    db.serialize(() => {
      db.all(`SELECT id, target FROM targets ORDER BY target`, (err, unnormalized_rows) => {
        if (err) {
          reject(err);
        }

        const rows = unnormalized_rows.map((r) => {
          return { "id": r.id, "target": normalizeTarget(r.target) }
        });

        rows.sort((a, b) => indexOf[a.target] - indexOf[b.target]);

        resolve([
          rows.map((r) => r.id),
          rows.map((r) => r.target),
          Object.fromEntries(rows.map((r, i) => [r.target, i])),
          Object.fromEntries(rows.map((r) => [r.id, r.target])),
        ]);
      });
    });
  });
}

function fetchTarget(target_id) {
  return new Promise((resolve, reject) => {
    db.serialize(() => {
      db.get(`SELECT target FROM targets WHERE id = ${target_id}`, (err, row) => {
        if (err) {
          reject(err);
        }
        resolve(row.target);
      });
    });
  });
}

function readInFile(input_filename) {
  return new Promise((resolve, reject) => {
    fs.readFile(input_filename, { encoding: "utf-8" }, (err, data) => {
      if (err) {
        reject(err);
      }

      resolve(data);
    });
  });
}

function findNewline(data, offset = 0) {
  let i = offset;
  while (i < data.length) {
    if (data.at(i) == '\n'.charCodeAt(0)) {
      break;
    }
    i += 1;
  }

  if (i === data.length) {
    throw "up";
  }

  return i;
}

function parseOutFile(input_filename) {
  const UINT32_SIZE = 4;

  return new Promise((resolve, reject) => {
    fs.readFile(input_filename + ".out", { encoding: null }, (err, data) => {
      if (err) {
        reject(err)
      }

      const decoder = new TextDecoder("utf-8");

      const firstLineEnd = findNewline(data, 0);
      const configs = decoder.decode(data.buffer.slice(0, firstLineEnd)).split(":");

      const secondLineEnd = findNewline(data, firstLineEnd + 1);
      const goldParsers = decoder.decode(data.buffer.slice(firstLineEnd + 1, secondLineEnd)).split(":");

      let outputs = [];

      let ptr = secondLineEnd + 1;
      for (let config of configs) {
        const size = data.readUInt32LE(ptr);
        ptr += UINT32_SIZE;
        // we ignore the second part of the 64 bit header
        ptr += UINT32_SIZE;

        const payload = data.buffer.slice(ptr, ptr + size);
        outputs.push({
          target: config,
          chars: decoder.decode(payload)
        });

        ptr = ptr + size;
      }

      resolve([["0xdeadbeef"], [data], outputs]);
    });
  });
}

function splitConfig(config) {
  if (config.endsWith(".cfg")) {
    config = config.slice(0, -(".cfg".length));
  }

  const splits = config.split("/");
  const fullname = splits[splits.length - 1];

  const fullname_split = fullname.split("-");
  const language = fullname_split[0];
  const basename = fullname_split.slice(1).join("-");

  return {
    language: language,
    fullname: fullname,
    basename: basename
  };
}

function normalizeTarget(target) {
  const configs = target.split(".cfg-");
  return configs.map((p) => splitConfig(p).fullname).join("_");
}

function printableTarget(target) {
  return target;
  const dict = {
    "c-ccan": "ccan",
    "c-cjson": "cjson",
    "c-frozen": "frozen",
    "c-jansson": "jansson",
    "c-jsmn": "jsmn",
    "c-json-c": "json-c",
    "c-json-parser": "json-parser",
    "c-jsonh": "jsonh",
    "c-libjson": "libjson",
    "c-poco": "poco",
    "c-tiny-json": "tiny-json",
    "c-yajl": "yajl",
    "cpp-boost": "boost",
    "cpp-json": "nlohmann",
    "cpp-jsoncpp": "jsoncpp",
    "cpp-rapidjson": "rapidjson",
    "cpp-spidermonkey": "spider",
    "cpp-v8": "v8",
    "java-gson": "gson",
    "java-jackson": "jackson",
    "py-json": "py-json",
    "py-orjson": "or",
    "py-simplejson": "simple",
    "rust-serde": "serde"
  };

  console.log(target, dict[target]);

  return dict[target];
}
