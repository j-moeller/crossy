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
// - Original EdgeCoverageInstrumentor.kt
// - rename package
// - converted to Java from Kotlin

package com.paco.agent;

import org.jacoco.core.analysis.Analyzer;
import org.jacoco.core.analysis.ICoverageVisitor;
import org.jacoco.core.data.ExecutionDataStore;
import org.jacoco.core.internal.flow.ClassProbesAdapter;
import org.jacoco.core.internal.flow.ClassProbesVisitor;
import org.jacoco.core.internal.flow.IClassProbesAdapterFactory;
import org.jacoco.core.internal.instr.ClassInstrumenter;
import org.jacoco.core.internal.instr.IProbeArrayStrategy;
import org.jacoco.core.internal.instr.IProbeInserterFactory;
import org.jacoco.core.internal.instr.InstrSupport;
import org.jacoco.core.internal.instr.ProbeInserter;
import org.objectweb.asm.ClassReader;
import org.objectweb.asm.ClassVisitor;
import org.objectweb.asm.ClassWriter;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

import java.lang.invoke.MethodHandle;
import java.lang.invoke.MethodHandles;
import java.lang.invoke.MethodType;

import java.io.IOException;

public class EdgeCoverageInstrumentor implements Instrumentor {
    private EdgeCoverageStrategy strategy;
    private Class<?> coverageMapClass;
    private int initialEdgeId;
    private int nextEdgeId;
    private String coverageMapInternalClassName;
    private MethodHandle enlargeIfNeeded;

    private IProbeInserterFactory edgeCoverageProbeInserterFactory = (access,
            name,
            desc, mv, arrayStrategy) -> new EdgeCoverageProbeInserter(access, name, desc, mv, arrayStrategy);
    private IClassProbesAdapterFactory edgeCoverageClassProbesAdapterFactory = new IClassProbesAdapterFactory() {
        @Override
        public ClassProbesAdapter makeClassProbesAdapter(ClassProbesVisitor cv, boolean trackFrames) {
            return new EdgeCoverageClassProbesAdapter(cv, trackFrames);
        }
    };
    private IProbeArrayStrategy edgeCoverageProbeArrayStrategy = new IProbeArrayStrategy() {
        @Override
        public int storeInstance(MethodVisitor mv,
                boolean clinit,
                int variable) {
            strategy.loadLocalVariable(mv, variable,
                    coverageMapInternalClassName);
            return strategy.getLoadLocalVariableStackSize();
        }

        @Override
        public void addMembers(ClassVisitor cv,
                int probeCount) {

        }
    };

    public EdgeCoverageInstrumentor(EdgeCoverageStrategy edgeCoverageStrategy, Class<?> coverageMap,
            int initialEdgeId) {
        this.strategy = edgeCoverageStrategy;
        this.coverageMapClass = coverageMap;
        this.initialEdgeId = initialEdgeId;
        this.nextEdgeId = initialEdgeId;

        coverageMapInternalClassName = coverageMapClass.getName().replace('.',
                '/');
        try {
            MethodHandles.Lookup lookup = MethodHandles.lookup();
            MethodType mt = MethodType.methodType(void.class, int.class);
            enlargeIfNeeded = lookup.findStatic(coverageMap, "enlargeIfNeeded",
                    mt);
        } catch (NoSuchMethodException e) {
            System.err.println("Cannot find enlargeIfNeeded");
            System.exit(-1);
        } catch (IllegalAccessException e) {
            System.err.println("Cannot find enlargeIfNeeded");
            System.exit(-1);
        }
    }

    @Override
    public byte[] instrument(byte[] bytecode) {
        // System.err.println(bytecode.length);
        ClassReader reader = InstrSupport.classReaderFor(bytecode);
        ClassWriter writer = new ClassWriter(reader, 0);
        int version = InstrSupport.getMajorVersion(reader);
        // System.err.println("api: " + InstrSupport.ASM_API_VERSION + " == " + Opcodes.ASM9);
        ClassInstrumenter classInstrumenter = new ClassInstrumenter(edgeCoverageProbeArrayStrategy,
                edgeCoverageProbeInserterFactory, writer);
        ClassVisitor visitor = new EdgeCoverageClassProbesAdapter(
                classInstrumenter,
                InstrSupport.needsFrames(version));
        reader.accept(visitor, ClassReader.EXPAND_FRAMES);
        return writer.toByteArray();
    }

    void analyze(ExecutionDataStore executionData, ICoverageVisitor coverageVisitor, byte[] bytecode,
            String internalClassName) {
        Analyzer analyzer = new Analyzer(executionData, coverageVisitor, edgeCoverageClassProbesAdapterFactory);
        try {
            analyzer.analyzeClass(bytecode, internalClassName);
        } catch (IOException e) {
            // TODO Auto-generated catch block
            e.printStackTrace();
        }
    }

    public int numEdges() {
        return nextEdgeId - initialEdgeId;
    }

    private int nextEdgeId() {
        try {
            enlargeIfNeeded.invokeExact(nextEdgeId);
        } catch (Throwable e) {
            System.err.println(e.getMessage());
            e.printStackTrace();
            System.exit(-1);
        }
        return nextEdgeId++;
    }

    /**
     * A [ProbeInserter] that injects bytecode
     * instrumentation at every control flow edge and
     * modifies the stack size and number of local
     * variables accordingly.
     */
    private class EdgeCoverageProbeInserter extends ProbeInserter {

        public EdgeCoverageProbeInserter(int access,
                String name,
                String desc,
                MethodVisitor mv, IProbeArrayStrategy arrayStrategy) {
            super(access, name, desc, mv, arrayStrategy);
        }

        @Override
        public void insertProbe(int id) {
            strategy.instrumentControlFlowEdge(mv, id,
                    variable, coverageMapInternalClassName);
        }

        @Override
        public void visitMaxs(int maxStack, int maxLocals) {
            int newMaxStack = Math.max(maxStack + strategy.getInstrumentControlFlowEdgeStackSize(),
                    strategy.getLoadLocalVariableStackSize());
            int newMaxLocals = maxLocals;
            if (strategy.getLocalVariableType() != null) {
                newMaxLocals++;
            }
            mv.visitMaxs(newMaxStack, newMaxLocals);
        }

        @Override
        protected Object getLocalVariableType() {
            return strategy.getLocalVariableType();
        }
    }

    private class EdgeCoverageClassProbesAdapter extends ClassProbesAdapter {
        private ClassProbesVisitor cpv;

        public EdgeCoverageClassProbesAdapter(ClassProbesVisitor cpv, boolean trackFrames) {
            super(cpv, trackFrames);
            this.cpv = cpv;
        }

        @Override
        public int nextId() {
            return nextEdgeId();
        }

        @Override
        public void visitEnd() {
            cpv.visitTotalProbeCount(numEdges());
            cpv.visitEnd();
        }
    }
}
