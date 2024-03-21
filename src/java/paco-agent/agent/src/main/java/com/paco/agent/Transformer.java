// Copyright 2021 Code Intelligence GmbH
// Modifications Copyright 2022 Jan Niklas Drescher
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 
// Modifications:
// - Original: RuntimeInstrumentor.kt
// - rename package
// - rename class
// - convert from Kotlin to Java

package com.paco.agent;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.IllegalClassFormatException;
import java.lang.instrument.Instrumentation;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.security.ProtectionDomain;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;
import java.util.Set;

import com.paco.runtime.CoverageMap;

import static java.util.Collections.emptyMap;
import static java.util.Collections.emptySet;

public class Transformer implements ClassFileTransformer {
    public Class<?> coverageMapClass = CoverageMap.class;
    private Path dumpClassesDir = Paths.get("").resolve("classes");
    private Instrumentation instrumentation;
    private List<String> blockList = Arrays.asList("com/paco", "java/", "javax/", "jdk/", "sun/", "com/sun/", "kotlin/", "org/junit/", "\\[");

    private CoverageIdStrategy coverageIdSynchronizer = new MemSyncedCoverageIdStrategy();

    public Transformer(Instrumentation instrumentation) {
        this.instrumentation = instrumentation;
    }

    private boolean doNotTransform(String className){
        return blockList.stream().anyMatch(prefix -> className.startsWith(prefix));
    }

    @Override
    public byte[] transform(ClassLoader loader, String className, Class<?> classBeingRedefined,
            ProtectionDomain protectionDomain, byte[] classfileBuffer) throws IllegalClassFormatException {
        if (doNotTransform(className)) {
            // prevent instrumenting paco and java lib code
            return null;
        }

        // System.err.println("Instrumenting: " + className);

        try {
            byte[] instrumentedByteCode = instrument(className, classfileBuffer);
            if (instrumentedByteCode == null) {
                System.err.println("NULL!");
            } else {
                // System.out.println("Instrumentation success");
            }

            if (instrumentedByteCode == null) {
                System.err.println("Error instrumenting: " + className);
            } else {
                // dumpToClassFile(className, instrumentedByteCode);
                // dumpToClassFile(className, classfileBuffer, ".original");
            }

            return instrumentedByteCode;
        } catch (Throwable t) {
            System.err.println(t);
            t.printStackTrace(System.err);
            return null;
        }

    }

    @Override
    public byte[] transform(Module module, ClassLoader loader, String className, Class<?> classBeingRedefined,
            ProtectionDomain protectionDomain, byte[] classfileBuffer) throws IllegalClassFormatException {
        try {
            if (module != null && !module.canRead(Transformer.class.getModule())) {
                if (!instrumentation.isModifiableModule(module)) {
                    System.err.println("WARN: Failed to instrument " + className + " in unmodifiable module " + module.getName() + ", skipping");
                    return null;
                }
                // System.out.println("redefineModule");
                Set<Module> extraReads = Collections.singleton(Transformer.class.getModule());
                instrumentation.redefineModule(module, extraReads, emptyMap(),
                        emptyMap(), emptySet(), emptyMap());
            }
            return transform(loader, className, classBeingRedefined, protectionDomain, classfileBuffer);
        } catch (Throwable t) {
            System.out.println(t);
            t.printStackTrace();
            return null;
        }

    }

    private void dumpToClassFile(String className, byte[] classfileBuffer) {
        String filename = className.replace("/", ".") + ".class";
        Path absolutePath = dumpClassesDir.toAbsolutePath().resolve(filename);
        File dumpFile = absolutePath.toFile();
        try {
            dumpFile.createNewFile();
            FileOutputStream writer = new FileOutputStream(dumpFile);
            writer.write(classfileBuffer);
            writer.close();
        } catch (IOException e) {
            System.err.println("Error: " + e);
            e.printStackTrace();
        }
    }

    private void dumpToClassFile(String className, byte[] classfileBuffer, String suffix) {
        dumpToClassFile(className + suffix, classfileBuffer);
    }

    private byte[] instrument(String className, byte[] classfileBuffer) {
        ClassInstrumentor instrumentor = new ClassInstrumentor(classfileBuffer);
        coverageIdSynchronizer.withIdForClass(className, (int x) -> (instrumentor.coverage(x)));
        return instrumentor.getInstrumentedBytecode();
    }
}
