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
// - Original: ClassInstrumentor.kt
// - rename package
// - convert from Kotlin to Java
// - remove all instrumentations except edge coverage

package com.paco.agent;

import com.paco.runtime.CoverageMap;

// copies Jazzers ClassInstrumentor, only keeping coverage
public class ClassInstrumentor {
    private byte[] instrumentedBytecode;
    private Class<?> coverageMapClass = CoverageMap.class;

    public ClassInstrumentor(byte[] bytecode) {
        this.instrumentedBytecode = bytecode;
    }

    public int coverage(int initialEdgeId) {
        // System.out.println("Adding coverage with initialEdgeId: " + initialEdgeId);
        EdgeCoverageStrategy edgeCoverageStrategy = new EdgeCoverageStrategy();
        EdgeCoverageInstrumentor edgeCoverageInstrumentor = new EdgeCoverageInstrumentor(edgeCoverageStrategy,
                coverageMapClass, initialEdgeId);
        this.instrumentedBytecode = edgeCoverageInstrumentor.instrument(instrumentedBytecode);
        if (this.instrumentedBytecode == null) {
            System.err.println("Instrumentation failed");
        }
        // System.out.println("Instrumented " + edgeCoverageInstrumentor.numEdges() + " edges");
        return edgeCoverageInstrumentor.numEdges();
    }

    public byte[] getInstrumentedBytecode() {
        return this.instrumentedBytecode;
    }
}
