# EBLT - Extended Blueprint Library Test Plugin for Unreal Engine 

[![MIT License](https://img.shields.io/github/license/bUsernameIsUnavailable/BLT?style=for-the-badge)](https://github.com/bUsernameIsUnavailable/BLT/blob/master/LICENSE.md)


Developed with Unreal Engine 5.1.1, but should be compatible with newer versions like Unreal Engine 5.3. The assets used for defininig the level are based on the HoursOfCode learning tutorial from Epic marketplace, publicly available for learning purposes. To satisfy our evaluation objectives, we highly modified the level, some rules and placed new objects in different positions.

Make sure to check the documentation folder https://github.com/AGAPIA/EBLT/tree/master/Documentation for the full technical paper (that best paper award at ICSOFT 2023 https://icsoft.scitevents.org/PreviousAwards.aspx),  presentation slides, and Video demo with explanations.

## Quick Evaluation

To try a release version to see the overall production capabilities of our framework, download from the following link https://drive.google.com/file/d/19W-9tCOzcUm1-E2kU0sTdBUHnyyniQoi/view?usp=sharing, unzip then run the executable in the folder.

The tests that run by default are in the annotation file https://github.com/AGAPIA/EBLT/blob/master/Plugins/Blt/Content/images/AnnotationsExample2.json. 
If you open the file you note that there are two cases in there, a TEST and a TUNE case.  A snapshot for TEST from the file mentioned above is given in the picture below. ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/AnnotationsFile2.png?raw=true).
The concrete use-case of this definition is a general-purpose pathfinding system, trying to start from different positions and getting to a marked location in the level.

Each has different ranges and sets to control the following variables:
 - A range of scales for the character (CharacterScale)
 - A set of spawn positions (StartLocation)
 - A destination point or entity to arrive at (DestLocation_asEntity).
 - A range of maximum speeds for the main character (walkSpeed and sprintSpeed)
 - A range of jumping velocities (jumpVelocity)
 - A set of variables defining test case acceptance. E.g:
     - Getting to the destination in less than 30 seconds: timeToPathLimit.
     - The maximum limit of time for allowing the character to not move from its place (detecting blocked characters): movingIdleLimit.
 - The minimum number of times to run each individual case (numInstances) for sampling in the fuzzing process (numInstances)

 Given this, the range of values that can be chosen by the fuzzer is TotalRangeSize = |CharacterScale| x |StartLocation| x |DestLocation_asEntity| x |walkSpeed| x |sprintSpeed| x |jumpVelocity|. 
 * From the viewpoint of the TEST case, this total range of values should be correct. The fuzzer will try to find samples of values that do not satisfy the correctness of the test.
 * From the viewpoint of the TUNE case, it will try to find values in this range that satisfy the correctness criteria. It is the inverse process to testing, very used to find capabilities that satisfy goals in the game development or simulation engine industries.

Remember that everything is flexible and modular! Variables can be customized by the user (the next sections will show the connection between the variables and the game itself). Characters, tests, and levels can be adjusted and recreated as needed.

While running the tests you will see a bunch of output logs on the screen for each requested test and iteration as specified in the annotations file:![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_run_output_1.png?raw=true)

 The output folders contain different csv files for each group in the annotation file.
    - For the testing items: the set of parameters that determined a failure test.
    - For the tuning items: the set of parameters that succeeded in accomplishing the desired behavior.
![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_run_output_2.png?raw=true)



## Setup notes - Checking and contributing to the plugin and its capabilities in source code and engine.


### Trying the demo level and plugin in editor
1. Install the Engine from here https://www.unrealengine.com/en-US/unreal-engine-5 
2. Clone or download the repository
3. Then, you can click the Launch Game option to test it, or generate a solution and contribute to the project!
  ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_1.png?raw=true)
  Note: You can use Visual Studio 2022 Community Edition or Rider from https://www.jetbrains.com/lp/rider-unreal/  to extend the project or view the source code.
4. Once the editor is opened, click the green button like in the image below and the tests will start.
   ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_run.png?raw=true)
   

### Trying the plugin only in your own project

For trying the plugin in your own project:
1. Copy the content of the plugin only, https://github.com/AGAPIA/EBLT/tree/master/Plugins/Blt, to your project's Plugins directory.
2. Open the editor of your project
3. Go to Edit/Plugins and search for EBLT
4. Enable the plugin as shown in the figure below:

![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_pluginconfig.png?raw=true)  

### Tests, annotations and setup for your own project

First, drag the EBLTManager component to the world, doesn't matter the position or anything:

![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_drag_manager.png?raw=true)   

#### To define and register a new test/tune task:

The tests are defined with the BDD methodology in mind. https://en.wikipedia.org/wiki/Behavior-driven_development 
1. Derive a blueprint from EBLTTestTemplate (see full paper for details about this custom plugin class)

2. Set the type of the purpose case, i.e., TEST or TUNE, then make an annotation file similar to the one in Annotations2.json with a correspondence to a variable in the blueprint definition of the test as shown in the picture below. This is basically the test running in the demo and annotation given as example.
![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_level_testSetup_fuzzingTest.png?raw=true)

3. Define your own GIVEN, WHEN, THEN clauses according to the BDD methodology in a graphical blueprints format. As shown in the picture (our example in the demo) below the correspondencies are the following: GIVEN - SetupContext, WHEN - CheckTriggers, THEN - CheckTestCorrectness.

![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_bdt.png?raw=true)

4. Register your new test to the manager, by adding them to the Allowed Test Instances array. Check in the picture below how we register the two cases, test and tune examples. Remember that the number of instance running consecutively for the same task is defined in the annotations file in the numInstances variable.

![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/images/setup_tests_to_manager.png?raw=true)

5. To restart the testing and/or tuning specified tasks and run continuously until forever (or until there is a technical problem..), just enable the variable M Continuous Test Running show in the figure above, below the Allowed Test  Instances array.

## Remember to give us feedback and can't wait for your contributions.

To properly cite our work:

@inproceedings{DBLP:conf/icsoft/PaduraruCS23a,
  author       = {Ciprian Paduraru and
                  Rares Cristea and
                  Alin Stefanescu},
  editor       = {Hans{-}Georg Fill and
                  Francisco Jos{\'{e}} Dom{\'{\i}}nguez Mayo and
                  Marten van Sinderen and
                  Leszek A. Maciaszek},
  title        = {Automatic Fuzz Testing and Tuning Tools for Software Blueprints},
  booktitle    = {Proceedings of the 18th International Conference on Software Technologies,
                  {ICSOFT} 2023, Rome, Italy, July 10-12, 2023},
  pages        = {151--162},
  publisher    = {{SCITEPRESS}},
  year         = {2023},
  url          = {https://doi.org/10.5220/0012121900003538},
  doi          = {10.5220/0012121900003538},
  timestamp    = {Sat, 30 Sep 2023 09:46:57 +0200},
  biburl       = {https://dblp.org/rec/conf/icsoft/PaduraruCS23a.bib},
  bibsource    = {dblp computer science bibliography, https://dblp.org}
}

