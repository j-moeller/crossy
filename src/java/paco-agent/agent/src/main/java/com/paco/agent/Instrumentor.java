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
// - Original: Instrumentor.kt
// - rename package
// - convert to Java from Kotlin
// - remove enum InstrumentationType

package com.paco.agent;

import org.objectweb.asm.Opcodes;
import org.objectweb.asm.tree.MethodNode;

// java conversion of Jazzers Instrumentor.kt without the InstrumentationType
public interface Instrumentor {
    final static int ASM_API_VERSION = Opcodes.ASM9;

    byte[] instrument(byte[] bytecode);

    default boolean shouldInstrument(int access) {
        return ((access & Opcodes.ACC_ABSTRACT) == 0) &&
                ((access & Opcodes.ACC_NATIVE) == 0);
    }

    default boolean shouldInstrument(MethodNode method) {
        return shouldInstrument(method.access) &&
                method.instructions.size() > 0;
    }
}
