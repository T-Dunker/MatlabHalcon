# MatlabHalcon
Matlab wrapper for MVTec Halcon functions with XLD serialization.

A modification of the MVTec Halcon wrapper by Dirk-Jan Kroon permitting to use functions with HObject of type XLD as input or return value.

This MVTec Halcon wrapper can pass HObjects of type XLD between functions. The XLD is not mapped to a Matlab structure. It is simply serialized to a byte stream.

The build process is packed into the function build(). Executing build() will create a new folder "../halcon-XXX-YYY-ZZZ/+halcon", where XXX stands for the Halcon version, YYY for the matlab('arch') and ZZZ for the short name of the mex C++ compiler.

After adding the "halcon-XXX-YYY-ZZZ" folder to the matlab path one can e.g. use the model finding functions
```
xld = halcon.EdgesSubPix(templateImage, ...);
modelId = halcon.CreateScaledShapeModelXld(xld, ...);
[rows, cols, angles, scales, scores] = halcon.FindScaledShapeModel(searchImage, modelId, ...);
```


[![View MVTec Halcon wrapper with XLD serialization on File Exchange](https://www.mathworks.com/matlabcentral/images/matlab-file-exchange.svg)](https://de.mathworks.com/matlabcentral/fileexchange/70976-mvtec-halcon-wrapper-with-xld-serialization)
