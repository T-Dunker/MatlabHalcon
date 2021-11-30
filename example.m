%% add install folder to path if needed
pathList = strsplit(path, pathsep);
installFolder = getDeployFolder();
check = cellfun(@(x) isequal(x, installFolder), pathList);
if ~any(check)
    addpath(installFolder);
end

%% L-shape
WidthHeight = [600 400];
Origin = [100 100];
x = Origin(1) + [0 400 400 100 100 0 0];
y = Origin(2) + [0 0 100 100 200 200 0];
TestImage = uint8(255*poly2mask(x, y, WidthHeight(2), WidthHeight(1)));
TestImage = imrotate(TestImage, 5, 'bilinear', 'crop');
TestImage = imtranslate(TestImage, [-21 -17]);
TestImage = imnoise(TestImage, 'gaussian');
imshow(TestImage);

%% a camera model that maps 0.001 m to one pixel
MeterPerPixel = .001;
ObjectDistance = 1;
CenterPoint = .5*(WidthHeight - 1);
PixelSize = 5e-6;
ImageDistance = ObjectDistance*PixelSize/MeterPerPixel;
CameraParam = [ImageDistance 0 PixelSize([1 1]) CenterPoint WidthHeight];

%% model contour in meter
% Matlab left upper corner is [1 1] in Halcon [0 0]
X = (x - Origin(1))*MeterPerPixel;
Y = (y - Origin(2))*MeterPerPixel;
O = (Origin - 1 - CenterPoint)*MeterPerPixel;
ShapeContour = halcon.GenContourPolygonXld(Y,X);
ReferencePose = [O ObjectDistance 0 0 0 2];

%% create a model
AngleStart = -.5;
AngleExtent = 1;
ScaleRMin = 1;
ScaleRMax = 1;
ScaleCMin = 1;
ScaleCMax = 1;
ModelId = halcon.CreatePlanarCalibDeformableModelXld(...
    ShapeContour, CameraParam, ReferencePose, 'auto', ...
    AngleStart, AngleExtent, 'auto', ...
    ScaleRMin, ScaleRMax, 'auto', ...
    ScaleCMin, ScaleCMax, 'auto', ...
    'none', 'ignore_local_polarity', 5, [], []);

%% find a model
MinScore = .8;
NumMatches = 1;
MaxOverlap = 0;
NumLevels = 0;
Greediness = .9;
% a rows x columns Matlab image is stored columnwise that is why it is
% passed as a columns x rows image to Halcon to save time - in most cases
% this is no problem that the meaning of rows and columns are exchanged,
% e.g. for symmetric filters like edge detection.
% In this example case one could adapt the camera model etc. Yet, if time
% is not a problem one can transpose all in- and output images.
[Pose, PoseCov, Score] = halcon.FindPlanarCalibDeformableModel(...
    TestImage', ModelId, ...
    AngleStart, AngleExtent, ...
    ScaleRMin, ScaleRMax, ...
    ScaleCMin, ScaleCMax, ...
    MinScore, ...
    NumMatches, ...
    MaxOverlap, ...
    NumLevels, ...
    Greediness, [], []);  

%% show result
FoundMat = reshape(halcon.PoseToHomMat3d(Pose), [4 3])';
FoundMat(4, 4) = 1;
ReferenceMat = reshape(halcon.PoseToHomMat3d(ReferencePose), [4 3])';
ReferenceMat(4, 4) = 1;
LengthCoord = .1;
Pnt = [X; Y];
Pnt(4, :) = 1;
Pnt = [[LengthCoord*eye(3,4); 1 1 1 1] Pnt];
PntRef = ReferenceMat*Pnt;
PntFound = FoundMat*Pnt;
[RowsRef, ColsRef] = halcon.Project3dPoint(...
    PntRef(1,:), PntRef(2,:), PntRef(3,:), CameraParam);
[RowsFound, ColsFound] = halcon.Project3dPoint(...
    PntFound(1,:), PntFound(2,:), PntFound(3,:), CameraParam);
% Matlab left upper corner is [1 1] in Halcon [0 0]
RowsRef = cell2mat(RowsRef) + 1;
ColsRef = cell2mat(ColsRef) + 1;
RowsFound = cell2mat(RowsFound) + 1;
ColsFound = cell2mat(ColsFound) + 1;
imshow(TestImage);
hold on;
plot(ColsRef(5:end), RowsRef(5:end), '--m', 'LineWidth', 2);
plot(ColsFound(5:end), RowsFound(5:end), '-c', 'LineWidth', 2);
plot(ColsFound([1 4]), RowsFound([1 4]), '-r', 'LineWidth', 4);
plot(ColsFound([2 4]), RowsFound([2 4]), '-g', 'LineWidth', 4);
plot(ColsFound([3 4]), RowsFound([3 4]), '-b', 'LineWidth', 4);
hold off;