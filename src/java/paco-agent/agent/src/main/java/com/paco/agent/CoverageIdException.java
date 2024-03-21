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
// - copy CoverageIdException class
// - convert to Java from Kotlin

package com.paco.agent;

public class CoverageIdException extends RuntimeException {
    public CoverageIdException() {
        super("Failed to synchronize coverage IDs");
    }

    public CoverageIdException(Throwable cause) {
        super("Failed to synchronize coverage IDs", cause);
    }
}
