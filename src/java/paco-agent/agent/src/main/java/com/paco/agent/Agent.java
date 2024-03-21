package com.paco.agent;

import java.lang.instrument.Instrumentation;

public class Agent {

    public static void premain(String agentArgs, Instrumentation inst) {
        inst.addTransformer(new Transformer(inst),false);
    }

    public static void agentmain(String agentArgs, Instrumentation inst) {
        System.err.println("Error: Agent should be invoked at JVM startup!");
    }
}
