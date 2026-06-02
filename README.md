## Overview
A simple ROS 2 tool to convert [SDF files](http://sdformat.org/) to [URDF](https://docs.ros.org/en/rolling/Tutorials/Intermediate/URDF/URDF-Main.html) using `urdfdom` and `sdformat_urdf`.
<br>
## Usage
After building, source your workspace and run:

```sh
ros2 run sdf_to_urdf sdf_to_urdf <input.sdf> <output.urdf>
```
- `<input.sdf>`: Path to the input SDF file
- `<output.urdf>`: Path to save the converted URDF file

SDF to URDF Convertor adapted to ROS2 Humble. 
Taking inspiration [andreasBihlmaier/sdf_to_urdf ](https://github.com/andreasBihlmaier/sdf_to_urdf), this SDF to URDF Convertor works with ROS2 Humble. <br>
Modifications were made to convert ROS1 to ROS2 and support for deprecated API's and functions of ROS1 SDF to URDF Convertors.<br><br>
Limitations are yet to be explored and tested.
