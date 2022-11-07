package sh.ball.audio;

import sh.ball.audio.effect.Effect;
import sh.ball.audio.effect.EffectType;
import sh.ball.audio.engine.AudioDevice;
import sh.ball.audio.engine.AudioInputListener;
import sh.ball.audio.midi.MidiListener;

import javax.sound.sampled.AudioInputStream;
import java.util.List;

public interface AudioPlayer<S> extends Runnable, MidiListener, AudioInputListener {

  void reset() throws Exception;

  void stop();

  boolean isPlaying();

  void setOctave(int octave);

  void setBackingMidiVolume(double scale);

  void setRelease(double decaySeconds);

  void setAttack(double attackSeconds);

  void addFrame(S frame);

  void addEffect(EffectType type, Effect effect);

  void removeEffect(EffectType type);

  void read(byte[] buffer) throws InterruptedException;

  void read(double[] buffer) throws InterruptedException;

  void startRecord();

  void setDevice(AudioDevice device);

  AudioDevice getDefaultDevice();

  AudioDevice getDevice();

  List<AudioDevice> devices();

  AudioInputStream stopRecord();

  void setBrightness(double brightness);

  void setThreshold(double doubleValue);
}
