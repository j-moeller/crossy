JAVA_HOME := /usr/lib/jvm/java-17-openjdk-amd64
JNI_INCLUDE := -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux 
JVM_LINK := $(JNI_INCLUDE) -L$(JAVA_HOME)/lib/server/ -Wl,-rpath=$(JAVA_HOME)/lib/server/ -ljvm

JAVAC := $(JAVA_HOME)/bin/javac
RUNTIME_DIR := src/java/paco-agent/agent/src/main/java/com/paco/runtime

JAVA_BUILD_DIR := build/java

# copy the paco-agent.jar to the root of JAVA_DIR
build/java/paco-agent.jar: src/java/paco-agent/agent/target/agent-1.0-SNAPSHOT-jar-with-dependencies.jar build/java/libconnector.so
	mkdir -p $(dir $@)
	cp $< $@

# compile paco-agent.jar using maven
src/java/paco-agent/agent/target/agent-1.0-SNAPSHOT-jar-with-dependencies.jar: src/java/paco-agent/agent/src/main/java/com/paco/agent/*.java src/java/paco-agent/agent/src/main/java/com/paco/runtime/*.java
	mkdir -p $(dir $@)
	cd src/java/paco-agent && mvn package -Dmaven.test.skip

# build the C header file required to export native functions
$(RUNTIME_DIR)/com_paco_runtime_CoverageMap.h: $(RUNTIME_DIR)/CoverageMap.java
	mkdir -p $(dir $@)
	$(JAVAC) -h $(RUNTIME_DIR) $^

# copy the native function header file
src/java/com_paco_runtime_CoverageMap.h: $(RUNTIME_DIR)/com_paco_runtime_CoverageMap.h
	mkdir -p $(dir $@)
	cp $< $@

# build the native library that is later imported by the JVM
build/java/libconnector.so: src/java/Connector.cpp src/java/Connector.h src/java/com_paco_runtime_CoverageMap.h 
	mkdir -p $(dir $@)
	$(CXX) -fPIC -shared $(JNI_INCLUDE) -lc -o $@ $<

# build the launcher that uses JNI to create a new JVM instance
build/java/jvm_launcher: src/java/Launcher.cpp src/java/Launcher.h
	mkdir -p $(dir $@)
	$(CXX) $(JVM_LINK) -g -O2 $< -o $@

java: build/java/jvm_launcher build/java/libconnector.so build/java/paco-agent.jar