
### test coverage

#### 1 build target

```
mkdir tinynet/utest/build
cd tinynet/utest/build
cmake ..
make
ctest
```

#### 2 generate a report

```
lcov --capture --directory /home/fx/MyNet/tinynet --output-file coverage.info

lcov --remove coverage.info '/usr/*' '*/boost/*' --output-file filtered_coverage.info

genhtml filtered_coverage.info --output-directory coverage_report
```