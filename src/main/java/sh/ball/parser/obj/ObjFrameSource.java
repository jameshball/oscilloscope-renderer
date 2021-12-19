package sh.ball.parser.obj;

import sh.ball.audio.FrameSource;
import sh.ball.engine.Camera;
import sh.ball.engine.Vector3;
import sh.ball.engine.WorldObject;
import sh.ball.shapes.Shape;

import java.util.List;

public class ObjFrameSource implements FrameSource<List<Shape>> {

  private final WorldObject object;
  private final Camera camera;

  private Vector3 baseRotation = new Vector3(Math.PI, Math.PI, 0);
  private Vector3 currentRotation = baseRotation;
  private Double rotateSpeed = 0.0;
  private boolean active = true;

  public ObjFrameSource(WorldObject object, Camera camera) {
    this.object = object;
    this.camera = camera;
  }

  @Override
  public List<Shape> next() {
    currentRotation = currentRotation.add(baseRotation.scale(rotateSpeed));
    if (rotateSpeed == 0) {
      object.setRotation(baseRotation);
    } else {
      object.setRotation(currentRotation);
    }
    return camera.draw(object);
  }

  @Override
  public boolean isActive() {
    return active;
  }

  @Override
  public void disable() {
    active = false;
  }

  @Override
  public void enable() {
    active = true;
  }

  // TODO: Refactor!
  @Override
  public void setFrameSettings(Object settings) {
    if (settings instanceof ObjFrameSettings obj) {
      if (obj.focalLength != null && camera.getFocalLength() != obj.focalLength) {
        camera.setFocalLength(obj.focalLength);
      }
      if (obj.cameraPos != null && camera.getPos() != obj.cameraPos) {
        camera.setPos(obj.cameraPos);
      }
      if (obj.baseRotation != null) {
        this.baseRotation = obj.baseRotation;
      }
      if (obj.currentRotation != null) {
        this.currentRotation = obj.currentRotation;
      }
      if (obj.rotateSpeed != null) {
        this.rotateSpeed = obj.rotateSpeed;
      }
      if (obj.resetRotation) {
        object.resetRotation();
      }
    }
  }

  // TODO: Refactor!
  @Override
  public Object getFrameSettings() {
    return ObjSettingsFactory.rotation(baseRotation, currentRotation);
  }
}
