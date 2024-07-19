package com.adapter;

import java.io.IOException;
import java.nio.charset.StandardCharsets;

import com.fasterxml.jackson.databind.*;
import com.fasterxml.jackson.core.JsonParseException;

public class JacksonTarget {
    public byte[] run (byte[] buf) throws JsonParseException, IOException {
        String jsonString = new String(buf, StandardCharsets.UTF_8);
        ObjectMapper mapper = new ObjectMapper();
        JsonNode node = mapper.readTree(jsonString);
        String outString = node.toString();
        return outString.getBytes(StandardCharsets.UTF_8);
    }
}
