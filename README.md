# EBLT - Extended Blueprint Library Test Plugin for Unreal Engine 

[![MIT License](https://img.shields.io/github/license/bUsernameIsUnavailable/BLT?style=for-the-badge)](https://github.com/bUsernameIsUnavailable/BLT/blob/master/LICENSE.md)

Developed with Unreal Engine 5.1.1, but should be compatible with newer versions like Unreal Engine 5.3 (RARES CONFIRM). 

## Setup notes

1. Install the Engine from here https://www.unrealengine.com/en-US/unreal-engine-5 
2. Clone or download the repository
3. Then, you can click the Launch Game option to test it, or generate a solution and contribute to the project!
  ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_1.png?raw=true)
  Note: You can use Visual Studio 2022 Community Edition or Rider from https://www.jetbrains.com/lp/rider-unreal/  to extend the project or view the source code.
4. Once the editor is opened, click on the green button like in the image below and the tests will start.
   ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_run.png?raw=true)
   
5. The tests that run by default are in the annotation file in![alt text](https://github.com/AGAPIA/EBLT/blob/master/Plugins/Blt/Content/AnnotationsExample2.json?raw=true)

    If you open the file you will see various properties, ranges, and sets available for fuzzing.
    Remember that everything is flexible and modular !
    Characters, tests, and properties can be extended depending on your own custom levels, characters, and needs.

 6. While running the tests you will see a bunch of output logs on the screen for each requested test and iterations as specified in the annotations file.

   ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_run_output_1.png?raw=true)

 8. The output folders contain different csv files for each group in the annotation file.

    - For the testing items: the set of parameters that determined a failure test.
    - For the tuning items: the set of parameters that succeded in accomplishing the desired behavior.
   
    ![alt text](https://github.com/AGAPIA/EBLT/blob/master/Documentation/setup_run_output_2.png?raw=true)
