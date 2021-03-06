#ifndef SCRIPT_H
#define SCRIPT_H

#include <vector>
#include <list>
#include <memory>
#include <stdexcept>
#if __GNUG__
#   include <tr1/memory>
#endif
#include <iostream>
#include <fstream>
#include <sstream>

#include "scenegraph.h"
#include "sgutils.h"
#include "frame.h"

using namespace std;
using namespace tr1;

class Script {
private:
    list<Frame> frames;
    list<Frame>::iterator iter;
    int currFrameNum;

    Script(list<Frame> initial_frames) {
      frames = initial_frames;
      iter = frames.begin();
      currFrameNum = 0;
    }

    /**
     * Returns whether the current frame is defined (i.e., if there are frames)
     * to be displayed.
     */
    bool defined() {
      if (frames.size() == 0) {
        iter = frames.begin();
        currFrameNum = 0;
        return false;
      }
      return (iter != frames.end());
    }

public:
  Script() {
    frames = list<Frame>();
    iter = frames.begin();
    currFrameNum = 0;
  }

  /**
   * Returns the number of stored keyframes.
   */
  int getNumberOfKeyframes() {
    return frames.size();
  }

  void advanceIter() {
    iter++;
    currFrameNum++;
    cout << "Advancing to frame " << currFrameNum << endl;
  }

  void regressIter() {
    iter--;
    currFrameNum--;
    cout << "Regressing to frame " << currFrameNum << endl;
  }

  /**
   * Returns true if we're not undefined or the last frame.
   */
  bool canAnimate() {
    return (defined() && currFrameNum < frames.size() - 2);
  }

  /**
   * Renders the current frame in the scene.
   */
  void showCurrentFrameInScene() {
    if (defined()) {
      iter->showFrameInScene();
    }
  }

  /**
   * Replace CurrentFrame with the current frame from the scene.
   */
  void replaceCurrentFrameFromScene(shared_ptr<SgRootNode> rootNode) {
    if (defined()) {
      Frame frame = Frame(rootNode);
      *iter = frame;
      iter->showFrameInScene();
    } else {
      createNewFrameFromSceneAfterCurrentFrame(rootNode);
    }
  }

  /**
   * Steps the current frame forward.
   */
  void advanceCurrentFrame() {
    if (defined() && currFrameNum < frames.size() - 1) {
      advanceIter();
      iter->showFrameInScene();
    }
  }

  /**
   * Steps the current frame backwards.
   */
  void regressCurrentFrame() {
    if (defined()) {
      if (iter != frames.begin()) {
        regressIter();
        iter->showFrameInScene();
      }
    }
  }

  /**
   * Sets the current frame to the beginning of the frames sequence.
   */
  void goToBeginning() {
    iter = frames.begin();
    currFrameNum = 0;
    if (defined()) {
      iter->showFrameInScene();
    }
  }

  /**
   * Deletes the current frame.
   *
   * If the current key frame is defined, delete the current key frame and do
   * the following:
   *   - If the list of frames is empty after the deletion, set the current
   *     key frame to undefined.
   *   - Otherwise:
   *     - If the deleted frame is not the first frame, set the current frame
   *       to the frame immediately before the deleted frame.
   *     - Else set the current frame to the frame immediately after the
   *       deleted frame.
   */
  void deleteCurrentFrame() {
    if (defined()) {
      list<Frame>::iterator temp = iter;

      if (iter == frames.begin()) {
        /* Deleting the first frame, so result is set to the next frame. */
        temp++;
      } else {
        /* Deleting a non-first frame, so result is set to the prev. frame. */
        temp--;
      }
      frames.erase(iter);
      iter = temp;
      iter->showFrameInScene();
    }
  }

  /**
   * Creates a new frame (after the current frame) from the scene.
   *
   * If the current key frame is defined, create a new key frame immediately
   * after current key frame. Otherwise just create a new key frame. Copy
   * scene graph RBT data to the new key frame. Set the current key frame to
   * the newly created key frame.
   */
  void createNewFrameFromSceneAfterCurrentFrame(shared_ptr<SgRootNode> rootNode) {
    Frame frame = Frame(rootNode);

    if (defined()) {
      iter++;

      /* Insert inserts before current iterator position. */
      frames.insert(iter, frame);
      iter--;
      currFrameNum++;
    } else {
      frames.push_back(frame);
      iter = frames.begin();
      currFrameNum = 0;
    }

    cout << "Adding new frame (" << currFrameNum << ")" << endl;

    iter->showFrameInScene();
  }

  /**
   * Interpolates between the current frame and the next frame as if they were
   * alpha % between these two frames. Can't do this on the last frame.
   * @param alpha       The "weight" of the second frame relative to the first.
   *                    0 means the first frame is shown, 0 means the second
   *                    frame is shown. This should be between 0 and 1.
   */
  void interpolate(float alpha) {
    iter--;
    Frame prevFrame = *iter;
    iter++;

    Frame firstFrame = *iter;

    iter++;
    Frame secondFrame = *iter;
    iter++;
    Frame afterFrame = *iter;
    iter--;
    iter--;

    Frame::interpolate(prevFrame, firstFrame, secondFrame, afterFrame, alpha).showFrameInScene();
  }

  /**
   * Loads a script from the default file.
   */
  static Script loadScriptFromFile(const string filename, shared_ptr<SgRootNode> rootNode) {
    ifstream file;
    file.open(filename.c_str());
    vector<string> serialized = vector<string>();

    /* return an empty Script if there's no script file */
    if (file == NULL) {
      cout << "No script file found. Ensure that there is a file named " << filename << " in the same directory as the executable." << endl;
      return Script();
    }

    /* get the lines (of which each is a frame) from the file */
    string line;
    while (getline(file, line)) serialized.push_back(line);
    file.close();

    /* deserialize the frames */
    list<Frame> newFrames = list<Frame>();
    for (int i = 0; i < serialized.size(); i++) {
      newFrames.push_back(Frame::deserialize(rootNode, serialized[i]));
    }

    return Script(newFrames);
  }

  /**
   * Writes a script to the default file.
   */
  void writeScriptToFile(string filename) {
    ofstream file;
    file.open(filename.c_str());

    string serialized = "";
    for (list<Frame>::iterator it = frames.begin(); it != frames.end(); ++it) {
      serialized += it->serialize() + "\n";
    }

    file << serialized;
    file.close();
  }
};

#endif
