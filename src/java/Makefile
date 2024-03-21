JAVA_HOME := /usr/lib/jvm/java-17-openjdk-amd64
JNI_INCLUDE := -I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux 
JVM_LINK := $(JNI_INCLUDE) -L$(JAVA_HOME)/lib/server/ -Wl,-rpath=$(JAVA_HOME)/lib/server/ -ljvm

JAVAC := $(JAVA_HOME)/bin/javac
RUNTIME_DIR := $(JAVA_DIR)/paco-agent/agent/src/main/java/com/paco/runtime

JAVA_BUILD_DIR := $(BUILD_DIR)/java

.PHONY: clean-java java

# copy the paco-agent.jar to the root of JAVA_DIR
$(JAVA_BUILD_DIR)/paco-agent.jar: $(JAVA_DIR)/paco-agent/agent/target/agent-1.0-SNAPSHOT-jar-with-dependencies.jar $(JAVA_BUILD_DIR)/libconnector.so
	mkdir -p $(dir $@)
	cp $< $@

# compile paco-agent.jar using maven
$(JAVA_DIR)/paco-agent/agent/target/agent-1.0-SNAPSHOT-jar-with-dependencies.jar: $(JAVA_DIR)/paco-agent/agent/src/main/java/com/paco/agent/*.java $(JAVA_DIR)/paco-agent/agent/src/main/java/com/paco/runtime/*.java
	mkdir -p $(dir $@)
	cd $(JAVA_DIR)/paco-agent && mvn package -Dmaven.test.skip

# build the C header file required to export native functions
$(RUNTIME_DIR)/com_paco_runtime_CoverageMap.h: $(RUNTIME_DIR)/CoverageMap.java
	mkdir -p $(dir $@)
	$(JAVAC) -h $(RUNTIME_DIR) $^

# copy the native function header file
$(JAVA_DIR)/com_paco_runtime_CoverageMap.h: $(RUNTIME_DIR)/com_paco_runtime_CoverageMap.h
	mkdir -p $(dir $@)
	cp $< $@

# build the native library that is later imported by the JVM
$(JAVA_BUILD_DIR)/libconnector.so: $(JAVA_DIR)/Connector.cpp $(JAVA_DIR)/Connector.h $(JAVA_DIR)/com_paco_runtime_CoverageMap.h 
	mkdir -p $(dir $@)
	$(CXX) -fPIC -shared $(JNI_INCLUDE) -lc -o $@ $<

# build the launcher that uses JNI to create a new JVM instance
$(JAVA_BUILD_DIR)/jvm_launcher: $(JAVA_DIR)/Launcher.cpp $(JAVA_DIR)/Launcher.h
	mkdir -p $(dir $@)
	$(CXX) $(JVM_LINK) -g -O2 $< -o $@

java: $(JAVA_BUILD_DIR)/jvm_launcher $(JAVA_BUILD_DIR)/libconnector.so $(JAVA_BUILD_DIR)/paco-agent.jar

clean-java:
	rm -f $(JAVA_BUILD_DIR)/paco-agent.jar
	rm -f $(JAVA_BUILD_DIR)/libconnector.so
	rm -f $(JAVA_BUILD_DIR)/jvm_launcher
	cd $(JAVA_DIR)/paco-agent && mvn clean
	cd $(JAVA_DIR)/paco-agent/agent && mvn clean
	cd $(JAVA_DIR)/paco-agent/jacoco && mvn clean
