package audio;

import java.io.IOException;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import javax.xml.parsers.ParserConfigurationException;
import org.xml.sax.SAXException;
import parser.FileParser;
import shapes.Shape;
import shapes.Vector2;

public class AudioClient {

  private static final int BUFFER_SIZE = 20;

  private static final int SAMPLE_RATE = 192000;
  private static final float ROTATE_SPEED = 0;
  private static final float TRANSLATION_SPEED = 0;
  private static final Vector2 TRANSLATION = new Vector2(0, 0.5);
  private static final float SCALE = 1;
  private static final float WEIGHT = Shape.DEFAULT_WEIGHT;

  // args:
  // args[0] - path of .obj or .svg file
  // args[1] - rotation speed of object
  // args[2] - focal length of camera
  // args[3] - x position of camera
  // args[4] - y position of camera
  // args[5] - z position of camera
  //
  // example:
  // osci-render models/cube.obj 3
  public static void main(String[] programArgs)
      throws IOException, ParserConfigurationException, SAXException, InterruptedException {
    // TODO: Calculate weight of lines using depth.
    //  Reduce weight of lines drawn multiple times.
    //  Find intersections of lines to (possibly) improve line cleanup.
    //  Improve performance of line cleanup with a heuristic.

    AudioArgs args = new AudioArgs(programArgs);
    ArrayBlockingQueue<List<Shape>> frameQueue = new ArrayBlockingQueue<>(BUFFER_SIZE);

    System.out.println("Parsing " + args.filePath + "...");
    FileParser fileParser = args.getFileParser();
    System.out.println("Finished parsing");

    System.out.println("Connecting to audio player");
    AudioPlayer player = new AudioPlayer(SAMPLE_RATE, frameQueue, ROTATE_SPEED, TRANSLATION_SPEED,
        TRANSLATION, SCALE, WEIGHT);

    System.out.println("Starting audio stream");
    new Thread(player).start();

    while (true) {
      try {
        frameQueue.put(fileParser.nextFrame());
      } catch (InterruptedException e) {
        e.printStackTrace();
        System.out.println("Frame missed.");
      }
    }
  }
}