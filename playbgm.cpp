#include "playbgm.h"
#include <future>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <QDebug>
#include <signal.h>

pid_t PlayBgm::bgmPid = 0;
pid_t PlayBgm::correctPid = 0;
pid_t PlayBgm::wrongPid = 0;
pid_t PlayBgm::timerPid = 0;

const std::string PlayBgm::MUSIC_DIR = "/mnt/sd/music/";
const std::string PlayBgm::BGM = "bgm.wav";
const std::string PlayBgm::CORRECT = "correct.wav";
const std::string PlayBgm::WRONG = "wrong.wav";
const std::string PlayBgm::TIMER = "timer.wav";

PlayBgm::PlayBgm()
{ }

void PlayBgm::playOnLoop(const std::string& wavFileName)
{
    std::string musicFilePath = MUSIC_DIR + wavFileName;
    if (access(musicFilePath.c_str(), F_OK) == -1) {
        // if file is not existed
        return;
    }

    std::thread th(PlayBgm::inPlayOnLoop, wavFileName);
    th.detach();
}


void PlayBgm::playOnce(const std::string& wavFileName)
{
    std::string musicFilePath = MUSIC_DIR + wavFileName;
    if (access(musicFilePath.c_str(), F_OK) == -1) {
        // if file is not existed
        return;
    }

    std::thread th(PlayBgm::inPlayOnce, wavFileName);
    th.detach();
}

void PlayBgm::inPlayOnLoop(const std::string& wavFileName)
{
    int status = 0;
    pid_t& pid = PlayBgm::getPidType(wavFileName);

    std::string bgmPath = "/mnt/sd/music/" + wavFileName;
    char * const cmdPlayArgv[] = {
        "/usr/bin/aplay",
        "-D",
        "plughw:3,0",
        const_cast<char*>(bgmPath.c_str()),
        NULL
    };

    for (;;)
    {
        status = posix_spawn(&pid, "/usr/bin/aplay", NULL, NULL, cmdPlayArgv, NULL);
        if (status != 0)
        {
            qDebug() << "Posix spawn status is proper";
            break;
        }

        if (waitpid(pid, &status, 0) == -1)
        {
            qDebug() << "Cannot wait the process";
            break;
        }
    }

    // clera pid value
    pid = 0;

    return;
}

void PlayBgm::inPlayOnce(const std::string& wavFileName)
{
    int status = 0;
    pid_t& pid = PlayBgm::getPidType(wavFileName);

    std::string bgmPath = "/mnt/sd/music/" + wavFileName;
    char * const cmdPlayArgv[] = {
        "/usr/bin/aplay",
        "-D",
        "plughw:3,0",
        const_cast<char*>(bgmPath.c_str()),
        NULL
    };

    status = posix_spawn(&pid, "/usr/bin/aplay", NULL, NULL, cmdPlayArgv, NULL);
    if (status != 0)
    {
        qDebug() << "Posix spawn status is proper";
    }

    if (waitpid(pid, &status, 0) == -1)
    {
        qDebug() << "Cannot wait the process";
    }

    // clera pid value
    pid = 0;

    return;
}

void PlayBgm::stopPlay(const std::string& wavFileName)
{
    pid_t& targetPid = PlayBgm::getPidType(wavFileName);
    if (targetPid == 0)
    {
        qDebug() << "current bgm is not been running.";
        return;
    }

    int ret = kill(targetPid, SIGTERM);
    if (ret != 0)
    {
        qDebug() << "Target Pid is not killed: " << wavFileName.c_str();
    }
}

pid_t& PlayBgm::getPidType(const std::string &wavFileName)
{
    if (wavFileName == BGM)
    {
        return bgmPid;
    }
    else if (wavFileName == CORRECT)
    {
        return correctPid;
    }
    else if (wavFileName == WRONG)
    {
        return wrongPid;
    }
    else if (wavFileName == TIMER)
    {
        return timerPid;
    }
    else
    {
        qDebug() << "Undefined file name";
        // default as bgmPid
        return bgmPid;
    }
}
