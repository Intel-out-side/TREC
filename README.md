# TREC
@author : Soichiro

Compilation of programs for research activities.

//updated on 2020.02.29
Note: Please send an email to soichiro[at]vt.edu if anything is unclear.
Note2: All the source codes are also uploaded in https://github.com/Intel-out-side/TREC

- Contents
  - instruction.pdf (not included in GitHub repo)
  - shield_both_mounted (arduino project)
  - soem_test.c

- 1. shield_both_mounted
  - I was using platform-IO on Atom editor for Arduino IDE.
  - When you run this on the default Arduino IDE, all you need are:
    ./lib/CAN-BUS_shield library and ./src directory.

  << Project structure >>
  |_ .pio
  |_ include
  |_ lib
    |_ CAN-BUS_Shield -- library for CAN-bus shield v1.2
  |_ src
    |_ EasyCAT.h  -- header file for EasyCAT library
    |_ main.cpp   -- rename this "main.ino" when running on Arduino IDE
    |_ profile.h  -- profile settings for EasyCAT shield
    |_ util.h     -- helper functions for CAN-bus communication
  |_ test
  |_ other stuff

- 2. soem_test.c
  - Sample program to check the functionality the above system. This works as a ethercat master.
  - It is highly recommended to run this on Linux machine since configuration for
    Windows machines are heavily tedious.
    (Works on Ubuntu 18.04 LTS)

  << HOW TO RUN >>
  1. Clone "SOEM_master" from https://github.com/OpenEtherCATsociety/SOEM
  2. place "soem_test.c" in "SOEM_master/test/linux/simple_test" directory
  3. Go to "SOEM_master/" (root directory)
  4. In command line, run "mkdir build" -> "cd build" -> "cmake .." -> "make"
  5. Go to "SOEM_master/build/test/linux/slaveinfo"
  6. Run "sudo ./slaveinfo" and check the ID of your ethernet adapter
  6. Run "cd ../simple_test" (i.e. Go to SOEM_master/build/test/linux/simple_test)
  7. Run "sudo ./soem_test [your adapter's id]"
  8. It should display fx, fy, fz, tx, ty, tz measured by the F/T sensor
