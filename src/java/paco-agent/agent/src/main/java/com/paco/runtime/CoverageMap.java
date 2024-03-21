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
// - rename package
// - use ByteBuffer provided by native function instead of sun.misc.Unsafe
// - remove replayCoveredIds & getCoveredIds

package com.paco.runtime;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

/**
 * Represents the Java view on a libFuzzer 8 bit counter coverage map. By using
 * a direct ByteBuffer,
 * the counters are shared directly with native code.
 */
public class CoverageMap {
    
    static {
        System.loadLibrary("connector");
    }

    private static final int debugID = (int)(Math.random() * 1000);
    private static final int UINT_32_T_SIZE = 4;

    private static final ByteBuffer mbb = getCounterBuffer();
    private static final int MAX_NUM_COUNTERS = mbb.capacity() - UINT_32_T_SIZE;
    private static final int INITIAL_NUM_COUNTERS = 1 << 9;

    static {
        mbb.order(ByteOrder.LITTLE_ENDIAN);
        registerCounters(INITIAL_NUM_COUNTERS);
    }

    /**
     * The number of coverage counters that are currently registered with libFuzzer.
     * This number grows
     * dynamically as classes are instrumented and should be kept as low as possible
     * as libFuzzer has
     * to iterate over the whole map for every execution.
     */
    private static int currentNumCounters = INITIAL_NUM_COUNTERS;

    // Called via reflection.
    @SuppressWarnings("unused")
    public static void enlargeIfNeeded(int nextId) {
        int newNumCounters = currentNumCounters;
        while (nextId >= newNumCounters) {
            newNumCounters = 2 * newNumCounters;
            if (newNumCounters > MAX_NUM_COUNTERS) {
                System.out.printf("ERROR: Maximum number (%s) of coverage counters exceeded.",
                        MAX_NUM_COUNTERS);
                System.out.flush();
                System.exit(1);
            }
        }
        if (newNumCounters > currentNumCounters) {
            System.out.println("[CoverageMap" + debugID + "] Extend counters: " + currentNumCounters + " -> " + newNumCounters + " (nextID: " + nextId +")");
            System.out.flush();
            registerCounters(newNumCounters);
            currentNumCounters = newNumCounters;
        }
    }

    // Called by the coverage instrumentation.
    @SuppressWarnings("unused")
    public static void recordCoverage(final int id) { // calls are inserted by instrumentation
        final int idx = id + UINT_32_T_SIZE;
        final byte counter = mbb.get(idx);
        mbb.put(idx, (byte) (counter == -1 ? 1 : counter + 1));
    }

    private static void registerCounters(int newNumCounters) {
        System.out.println("[CoverageMap" + debugID + "] Register counters: " + newNumCounters);
        System.out.flush();

        mbb.putInt(0, newNumCounters);
    }

    private static native ByteBuffer getCounterBuffer();
}