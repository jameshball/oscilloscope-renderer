package shapes;

public abstract class Shape {

  public static final int DEFAULT_WEIGHT = 80;

  protected double weight = DEFAULT_WEIGHT;
  protected double length;

  public abstract Vector2 nextVector(double drawingProgress);

  public abstract Shape rotate(double theta);

  public abstract Shape scale(double factor);

  public abstract Shape scale(Vector2 vector);

  public abstract Shape translate(Vector2 vector);

  public abstract Shape setWeight(double weight);

  public double getWeight() {
    return weight;
  }

  public double getLength() {
    return length;
  }
}
