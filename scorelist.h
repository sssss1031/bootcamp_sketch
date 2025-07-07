#ifndef SCORELIST
#define SCORELIST

#include <vector>
#include <QString>
#include <QMetaType>

typedef std::vector<std::pair<QString, int>> ScoreList;
Q_DECLARE_METATYPE(ScoreList)

extern ScoreList g_pendingScoreList;
extern bool g_hasPendingScore;

#endif // SCORELIST

