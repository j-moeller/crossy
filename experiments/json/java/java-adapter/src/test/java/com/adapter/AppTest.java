package com.adapter;

import junit.framework.Test;
import junit.framework.TestCase;
import junit.framework.TestSuite;

import java.io.IOException;

/**
 * Unit test for simple App.
 */
public class AppTest
        extends TestCase {

    private String jsonString = "{\"firstName\":\"John\",\"lastName\":\"Smith\",\"isAlive\":true,\"age\":27,\"address\":{\"streetAddress\":\"212ndStreet\",\"city\":\"NewYork\",\"state\":\"NY\",\"postalCode\":\"10021-3100\"},\"phoneNumbers\":[{\"type\":\"home\",\"number\":\"212555-1234\"},{\"type\":\"office\",\"number\":\"646555-4567\"}],\"children\":[],\"spouse\":null}";

    /**
     * Create the test case
     *
     * @param testName name of the test case
     */
    public AppTest(String testName) {
        super(testName);
    }

    /**
     * @return the suite of tests being tested
     */
    public static Test suite() {
        return new TestSuite(AppTest.class);
    }

    public void testJackson() throws com.fasterxml.jackson.core.JsonParseException, IOException {
        JacksonTarget jacksonTarget = new JacksonTarget();
        byte[] result = jacksonTarget.run(jsonString.getBytes());
        assert (result != null);
    }

    public void testGson() throws com.google.gson.JsonParseException {
        GsonTarget gsonTarget = new GsonTarget();
        byte[] result = gsonTarget.run(jsonString.getBytes());
        assert (result != null);
    }
}
