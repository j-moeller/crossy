# Java-Adapter Maven Project

This project uses maven to build Java adapter classes.
Maven simplifies the dependency control.
Java version 17 must be used to be compatible to the fuzzer.
The Class [`JacksonAdapter`](./src/main/java/com/adapter/JacksonTarget.java) implements the function:
```java
public byte[] run (byte[] buf) throws JsonParseException, IOException
```
The matching fuzzer config file would be:
```
java
-
experiments/json/java/java-adapter/target/java-adapter-1.0.jar
com/adapter/JacksonTarget
run
```

The jar file can be build using `mvn package`.