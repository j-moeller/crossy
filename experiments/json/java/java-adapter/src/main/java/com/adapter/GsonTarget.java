package com.adapter;

import java.nio.charset.StandardCharsets;

import com.google.gson.Gson; 
import com.google.gson.JsonParseException;

public class GsonTarget {
    public byte[] run(byte[] buf) throws JsonParseException {
        String jsonString = new String(buf, StandardCharsets.UTF_8);
        Gson gson = new Gson();
        Object o = gson.fromJson(jsonString, Object.class);
        String output = gson.toJson(o);
        return output.getBytes(StandardCharsets.UTF_8);
    }
}
