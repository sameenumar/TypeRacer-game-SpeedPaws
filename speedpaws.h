#ifndef SPEEDPAWS_H
#define SPEEDPAWS_H

#include <QMainWindow>
#include <QTimer>
#include <QString>
#include <QWebSocket>
#include <QListWidgetItem>
#include "modeselectiondialog.h" // Include the dialog header file

QT_BEGIN_NAMESPACE
namespace Ui { class SpeedPaws; }
QT_END_NAMESPACE

class SpeedPaws : public QMainWindow
{
    Q_OBJECT

public:
    explicit SpeedPaws(QWidget *parent = nullptr);
    ~SpeedPaws();


protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private slots:
    void on_startbutton_clicked();
    // void on_replay_clicked();
    // void on_back_clicked();
    void validateTypedText();
    void validatePracticeTypedText();
    // void onModeSelected(const QString &mode);
    void startPracticeMode(const QString &mode);

    void on_connect_button_clicked();
    void onConnected();
    void onTextMessageReceived(const QString &message);
    void onDisconnected();  // Added missing declaration

    // Challenge and race handling
    void sendChallengeRequest(const QString &opponent);
    // void acceptChallenge(const QString &challenger);
    void startRace(const QJsonObject &raceData);  // Fixed parameter type to match usage
    void sendProgressUpdate();
    void sendJoinMessage();  // Added missing declaration
    void updatePlayerList(const QJsonArray &players);  // Added missing declaration
    void handleChallengeRequest(const QJsonObject &challenge);  // Added missing declaration
    void updateOpponentProgress(const QJsonObject &progress);  // Added missing declaration
    void onConnectionError(QAbstractSocket::SocketError error);


    void on_multiplayer_clicked();

    // void on_pushButton_clicked();

    void on_challenge_button_clicked();

    void on_practice_back_clicked();

    void on_practice_replay_clicked();

    void on_new_race_pressed();

    void on_result_back_pressed();

    void on_connect_back_clicked();

    void on_opponents_back_clicked();

private:
    Ui::SpeedPaws *ui;
    QWebSocket *webSocket;
    QString username;

    int playerProgress = 0;
    int practice_playerProgress = 0;


    int currentIndex = 0;
    int practice_currentIndex = 0;
    // Additions to track player and opponent names
    QString playerName;
    QString opponentName;

    // New methods to set names
    void setPlayerName(const QString &name);
    void setOpponentName(const QString &name);
    QString getPlayerName() const;
    QString getOpponentName() const;

    // Member variables to store initial car positions
    int player_1_initial_x_position;
    int player_1_initial_y_position;
    int player_2_initial_x_position;
    int player_2_initial_y_position;
    int practice_car_initial_x_position;
    int practice_car_initial_y_position;

    QString targettext;
    int elapsedtime;
    int countdown;
    QTimer *countdownTimer;
    QTimer *gameTimer;

    QString practice_targettext;
    int practice_elapsedtime;
    int practice_countdown;
    QTimer *practice_countdownTimer;
    QTimer *practice_gameTimer;
    // int currentIndex;


    QString getRandomParagraph(const QString &fileName);
    void startCountdown();
    void startGameTimer();
    void startPracticeGameTimer();
    void update_player_1_car_Position(int progress);
    void update_practice_car_Position(int progress);
    int calculateProgress(const QString &typed, const QString &typing);
    double calculateWPM(const QString &typed, int elapsedtime);
    double calculateAccuracy(const QString &typed, const QString &typing);
    void onTypingFinished();

    void onPracticeTypingFinished();
    void sendRaceFinished(int finalSpeed, int finalProgress, int timeTaken);
    void handleRaceResult(const QJsonObject &resultObj);

    void sendChallengeRejected(const QString &opponentName);

};

#endif // SPEEDPAWS_H

