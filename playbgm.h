#ifndef PLAYBGM_H
#define PLAYBGM_H

#include <string>

class PlayBgm
{
public:
    PlayBgm();

    static void playOnLoop(const std::string& wavFileName);
    static void playOnce(const std::string& wavFileName);
    static void stopPlay(const std::string& wavFileName);

    /* ------------------------------------------- */
    /* DEFINITION */
    static const std::string MUSIC_DIR;

    static const std::string BGM;
    static const std::string CORRECT;
    static const std::string WRONG;
    static const std::string TIMER;
    /* ------------------------------------------- */

private:
    static void inPlayOnLoop(const std::string& wavFileName);
    static void inPlayOnce(const std::string& wavFileName);
    static pid_t& getPidType(const std::string& wavFileName);

    static pid_t bgmPid;
    static pid_t correctPid;
    static pid_t wrongPid;
    static pid_t timerPid;
};

#endif // PLAYBGM_H
