// FROM https://gist.github.com/jewelsea/1463485 BY USER https://gist.github.com/jewelsea

package sh.ball.gui.components;

import com.sun.javafx.webkit.WebConsoleListener;
import javafx.concurrent.Worker;
import javafx.scene.layout.StackPane;
import javafx.scene.web.WebView;
import netscape.javascript.JSObject;

import java.io.IOException;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

public class CodeEditor extends StackPane {

  private WebView webview;

  private String editingCode = "";
  private String fileName = "";
  private BiConsumer<byte[], String> callback;

  private String applyEditingTemplate() throws URISyntaxException, IOException {
    String template = Files.readString(Path.of(getClass().getResource("/html/code_editor.html").toURI()));
    return template.replace("${code}", editingCode);
  }

  public void setCode(String editingCode, String fileName) {
    this.fileName = fileName;
    this.editingCode = editingCode;
    JSObject window = (JSObject) webview.getEngine().executeScript("window");
    window.setMember("newCode", editingCode);
    webview.getEngine().executeScript("editor.setValue(newCode);");
  }

  public void updateCode() throws Exception {
    editingCode = (String) webview.getEngine().executeScript("editor.getValue();");
    if (callback != null) {
      callback.accept(editingCode.getBytes(StandardCharsets.UTF_8), fileName);
    }
  }

  public void setCallback(BiConsumer<byte[], String> callback) {
    this.callback = callback;
  }

  public CodeEditor() {}

  public void initialize() throws URISyntaxException, IOException {
    webview = new WebView();
    webview.getEngine().getLoadWorker().stateProperty().addListener((e, old, state) -> {
      if (state == Worker.State.SUCCEEDED) {
        JSObject window = (JSObject) webview.getEngine().executeScript("window");
        window.setMember("javaCodeEditor", this);
      }
    });

    webview.getEngine().loadContent(applyEditingTemplate());

    WebConsoleListener.setDefaultListener((webView, message, lineNumber, sourceId) ->
      System.out.println(message + "[at " + lineNumber + "]")
    );

    this.getChildren().add(webview);
  }

  @FunctionalInterface
  public interface BiConsumer<T, U> {
    void accept(T t, U u) throws Exception;
  }
}