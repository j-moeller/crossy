// Copyright 2022 Code Intelligence GmbH
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
// - Original: StaticMethodStrategy.java & EdgeCoverageInstrumentor.kt
// - rename package
// - instead of using EdgeCoverageStrategy as an interface, implement StaticMethodStrategy directly in this class

package com.paco.agent;

import org.jacoco.core.internal.instr.InstrSupport;
import org.objectweb.asm.MethodVisitor;
import org.objectweb.asm.Opcodes;

public class EdgeCoverageStrategy {

    // adds calls to recordCoverage to edges
    public void instrumentControlFlowEdge(
            MethodVisitor mv, int edgeId, int variable, String coverageMapInternalClassName) {
        InstrSupport.push(mv, edgeId);
        mv.visitMethodInsn(
                Opcodes.INVOKESTATIC, coverageMapInternalClassName, "recordCoverage", "(I)V", false);
    }

    public int getInstrumentControlFlowEdgeStackSize() {
        return 1;
    }

    public Object getLocalVariableType() {
        return null;
    }

    public void loadLocalVariable(
            MethodVisitor mv, int variable, String coverageMapInternalClassName) {
    }

    public int getLoadLocalVariableStackSize() {
        return 0;
    }
}
