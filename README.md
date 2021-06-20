# L-system project
By Pol Martín Garcia

## Compilation
This project can be compiled using the provided `CMakeLists.txt` file. 

The only library dependence, not already provided by the project, is GLFW3.

In Ubuntu, GLFW3 can be easily installed with:
```bash
sudo apt-get install libglfw3
sudo apt-get install libglfw3-dev
```

Thus, the program can be compiled executed by running from the project directory:
```bash
mkdir build
cd build
cmake .. -CMAKE_BUILD_TYPE=Release
make
./l-system
```

## Execution
This program provides a UI that lets the user configure the model that wants to generate. **For more information about the usage, look into the Help window**.

The system supports:
* Multiple 3D operators
* Multiple rules
* Default and specific parameters for the operators
* Setup of constants to be used as parameters
* Stochastic L-system

Also, the software accepts different options to inspect the resulting model, such as:
* Interactible scene (camera movement)
* Different rendering modes (lines, and cylinders)
* Custom background and model color
* Enable/Disable antialiasing x4
* Model scaling

There are multiple examples avaiable to be loaded form the very same UI.