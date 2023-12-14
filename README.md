# OpenFBXQt
Lightweight engine for working with 3D graphics on Qt
- No Assimp or other additional dependencies
- Integrated with Qt 5 (can be adapted for Qt6)
- Skeletal animation
- FBX file support
- Works on Windows, Linux, macOS, Android, iOS and other

Viewer example:
<p align="center">
  <img src="misc/animation.gif">
</p>

# Usage
```
ofbxqt::Scene& scene = ui->sceneWidget->scene;
ofbxqt::OpenModelConfig config;
const ofbxqt::FileInfo fileInfo = scene.open("awesome_3d_model.fbx", config);
if (!fileInfo.topLevelModels.isEmpty())
{
    // here you can use 'fileInfo.topLevelModels' to access loaded objects
}
```
