#include "playbgm.h"
#include <future>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>
#include <QDebug>

static const std::string MUSIC_DIR = "/mnt/sd/music/";

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

void PlayBgm::inPlayOnLoop(const std::string& wavFileName)
{
    pid_t pid = 0;
    int status = 0;

    char * const cmdVolumeArgv[] = {
        "/usr/bin/amixer",
        "-c"
        "0",
        "cset",
        "numid=1",
        "1%",
        NULL
    };

    std::string bgmPath = "/mnt/sd/music/" + wavFileName;
    char * const cmdPlayArgv[] = {
        "/usr/bin/aplay",
        "-Dhw:3,0",
        const_cast<char*>(bgmPath.c_str()),
        NULL
    };

    status = posix_spawn(&pid, "/usr/bin/amixer", NULL, NULL, cmdVolumeArgv, NULL);
    if (status == 0)
    {
        if (waitpid(pid, &status, 0) == -1)
        {
            qDebug() << "posix_spawn_error 1 on volumeControl";
        }
    }

    for (;;)
    {
        status = posix_spawn(&pid, "/usr/bin/aplay", NULL, NULL, cmdPlayArgv, NULL);
        if (status == 0)
        {
            if (waitpid(pid, &status, 0) == -1)
            {
                qDebug() << "posix_spawn_error 1 on playBgmControl";
            }
        }
    }
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

void PlayBgm::inPlayOnce(const std::string& wavFileName)
{
    pid_t pid = 0;
    int status = 0;

    char * const cmdVolumeArgv[] = {
        "/usr/bin/amixer",
        "-c"
        "0",
        "cset",
        "numid=1",
        "100%",
        NULL
    };

    std::string bgmPath = "/mnt/sd/music/" + wavFileName;
    char * const cmdPlayArgv[] = {
        "/usr/bin/aplay",
        "-Dhw:3,0",
        const_cast<char*>(bgmPath.c_str()),
        NULL
    };

    status = posix_spawn(&pid, "/usr/bin/amixer", NULL, NULL, cmdVolumeArgv, NULL);
    if (status == 0)
    {
        if (waitpid(pid, &status, 0) == -1)
        {
            qDebug() << "posix_spawn_error 1 on volumeControl";
        }
    }
    else
    {
        qDebug() << "paosix_spawn error status: " << status;
    }

    qDebug() << "Music start";
    status = posix_spawn(&pid, "/usr/bin/aplay", NULL, NULL, cmdPlayArgv, NULL);
    if (status == 0)
    {
        if (waitpid(pid, &status, 0) == -1)
        {
            qDebug() << "posix_spawn_error 1 on playBgmControl";
        }
    }
    else
    {
        qDebug() << "paosix_spawn error status: " << status;
    }

    qDebug() << "Music end";
}
