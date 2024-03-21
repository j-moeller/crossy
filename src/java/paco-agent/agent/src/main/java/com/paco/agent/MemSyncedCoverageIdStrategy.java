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
// - Original Code at CoverageIdStrategy.kt
// - rename package
// - copy MemSyncedCoverageIdStrategy
// - convert to Java from Kotlin

package com.paco.agent;

import java.util.function.IntFunction;

/**
 * A memory synced strategy for coverage ID generation.
 *
 * This strategy uses a synchronized block to guard access to a global edge ID counter.
 * Even though concurrent fuzzing is not fully supported this strategy enables consistent coverage
 * IDs in case of concurrent class loading.
 *
 * It only prevents races within one VM instance.
 */
public class MemSyncedCoverageIdStrategy implements CoverageIdStrategy{
    private static int nextEdgeId = 0;

    @Override
    public synchronized void withIdForClass(String className,
                                            IntFunction<Integer> f) {
        nextEdgeId += f.apply(nextEdgeId);
    }


}
