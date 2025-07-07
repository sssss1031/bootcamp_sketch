#ifndef PLAYBGM_H
#define PLAYBGM_H

#include <string>

class PlayBgm
{
public:
    PlayBgm();

    static void playOnLoop(const std::string& wavFileName);
    static void playOnce(const std::string& wavFileName);

private:
    static void inPlayOnLoop(const std::string& wavFileName);
    static void inPlayOnce(const std::string& wavFileName);
};

#endif // PLAYBGM_H