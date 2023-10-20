# EBLT - Extended Blueprint Library Test Plugin for Unreal Engine 

[![MIT License](https://img.shields.io/github/license/bUsernameIsUnavailable/BLT?style=for-the-badge)](https://github.com/bUsernameIsUnavailable/BLT/blob/master/LICENSE.md)


Developed with Unreal Engine 5.1.1, but should be compatible with newer versions like Unreal Engine 5.3 (RARES CONFIRM). 
Check the documentation folder for the technical paper presented and won best paper award at ICSOFT + RARES ADD PLEASE
RARES adauga tu te rog si optiunea aia cu continuous run e important totusi cred

## Quick Evaluation

For trying a release version to see the overall production capabilities of our framework, download from the following link https://drive.google.com/file/d/19W-9tCOzcUm1-E2kU0sTdBUHnyyniQoi/view?usp=sharing, unzip then run the executable in the folder.

The tests that run by default are in the annotation file in![alt text](https://github.com/AGAPIA/EBLT/blob/master/Plugins/Blt/Content/AnnotationsExample2.json?raw=true). 
If you open the file you note that there are two case in there, a TEST and a TUNE case.  A snapshot for TEST from the file mentioned above is given in the picture below. ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/AnnotationsFile2.png?raw=true).
The concrete use-case of this definition is a general-purpose pathfinding system, trying to start from different positions and getting to a marked location in the level.

Each has different ranges and sets to control the following variables:
 - A set of spawn positions (Sp)
 - A range of scales for the character (Sc)
 - A range of maximum speeds for the main character (Rs)
 - A range of jumping velocities (Jv)
 - A set of variables defining test case acceptance
        - E.g., getting to the destination in less than 30 seconds: timeToPathLimit.
        - E.g., the maximum limit of time for allowing character to not move from its place (detecting blocked characters): movingIdleLimit.
 - The minimum number of times to run each individual case (numInstances) for sampling in the fuzzing process (numInstances)

 Given this, the range of values that can be chosed by the fuzzer is: TotalRangeSize = |Sp| x |Sc| x |Rs| x |Jv|. 
 * From the viewpoint of the TEST case, this total range of values should be correct. The fuzzer will try then to find samples of values that do not satisfy the correctness of the test.
 * From the viewpoint of the TUNE case, it will try to find values in this range that satisfy the correctness criteria. It is the inverse process to testing, very used to find capabilities that satisfy goals in the game development or simulation engines industries.

Remember that everything is flexible and modular ! Variables can be customized by the user (next sections will show the connection between the variables and the game itself). Characters, test, levels can be adjusted and recreating as needed.

While running the tests you will see a bunch of output logs on the screen for each requested test and iterations as specified in the annotations file: ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_run_output_1.png?raw=true)

 The output folders contain different csv files for each group in the annotation file.
    - For the testing items: the set of parameters that determined a failure test.
    - For the tuning items: the set of parameters that succeded in accomplishing the desired behavior.
![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_run_output_2.png?raw=true)



## Setup notes

1. Install the Engine from here https://www.unrealengine.com/en-US/unreal-engine-5 
2. Clone or download the repository
3. Then, you can click the Launch Game option to test it, or generate a solution and contribute to the project!
  ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_1.png?raw=true)
  Note: You can use Visual Studio 2022 Community Edition or Rider from https://www.jetbrains.com/lp/rider-unreal/  to extend the project or view the source code.
4. Once the editor is opened, click on the green button like in the image below and the tests will start.
   ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_run.png?raw=true)
   
5. 
