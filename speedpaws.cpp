#include "speedpaws.h"
#include "ui_speedpaws.h"
#include "modeselectiondialog.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QRandomGenerator>
#include <QDebug>
#include <QMessageBox>


SpeedPaws::SpeedPaws(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpeedPaws)
    , elapsedtime(0)
    , countdown(5)
    , practice_elapsedtime(0)
    , practice_countdown(5)
    , webSocket(new QWebSocket())

{
    ui->setupUi(this);
    ui->stackedWidget->setCurrentIndex(0);
    // Reset player car position at the start
    // ui->player_1_car->move(0, ui->player_1_car->y());
    ui->new_race->installEventFilter(this);
    ui->result_back->installEventFilter(this);

    connect(webSocket, &QWebSocket::errorOccurred, this, &SpeedPaws::onConnectionError);

    // Store initial positions of both cars
    player_1_initial_x_position = ui->player_1_car->x();  // Store initial x position of player 1's car
    player_1_initial_y_position = ui->player_1_car->y();  // Store initial y position of player 1's car
    player_2_initial_x_position = ui->player_2_car->x();  // Store initial x position of player 2's car
    player_2_initial_y_position = ui->player_2_car->y();  // Store initial y position of player 2's car
    practice_car_initial_x_position = ui->practice_car->x();  // Store initial x position of player 2's car
    practice_car_initial_y_position = ui->practice_car->y();


    ui->player_1_car->move(player_1_initial_x_position, player_1_initial_y_position);  // Start player 1's car at initial position
    ui->player_2_car->move(player_2_initial_x_position, player_2_initial_y_position);  // Start player 2's car at initial position
    ui->practice_car->move(practice_car_initial_x_position, practice_car_initial_y_position);

    ui->timerlabel->setText(" ");
    ui->my_result->setText("");
    ui->new_race->setEnabled(false);
    ui->result_back->setEnabled(false);
    // ui->startButton->setEnabled(false);
    // ui->stats->setVisible(false);
    // ui->back->setVisible(false);

    countdownTimer = new QTimer(this);

    connect(countdownTimer, &QTimer::timeout, this, [this]() {
        if (countdown > 0) {
            ui->countdownLabel->setText(QString("Pawtyping starts in: %1").arg(countdown));
            countdown--;
        } else {
            countdownTimer->stop();
            ui->countdownLabel->clear();
            // ui->back->setVisible(false);
            ui->typed->setReadOnly(false);
            startGameTimer();
            currentIndex=0;
        }
    });

    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, [this]() {
        elapsedtime++;
        int remainingTime = 60 - elapsedtime;
        if (remainingTime >= 0) {


            ui->timerlabel->setText(QString("%1").arg(remainingTime));
        } else {
            gameTimer->stop();
            onTypingFinished();
        }
    });

    practice_countdownTimer = new QTimer(this);
    connect(practice_countdownTimer, &QTimer::timeout, this, [this]() {
        if (practice_countdown > 0) {
            ui->practice_countdownLabel->setText(QString("Pawracing starts in: %1").arg(practice_countdown));
            practice_countdown--;
        } else {
            practice_countdownTimer->stop();
            ui->practice_countdownLabel->clear();
            // ui->practice_back->setVisible(false);
            ui->practice_typed->setReadOnly(false);
            startPracticeGameTimer();
            currentIndex = 0;
            if (practice_countdownTimer->isActive()) {
                practice_countdownTimer->stop();
            }
        }
    });

    practice_gameTimer = new QTimer(this);
    connect(practice_gameTimer, &QTimer::timeout, this, [this]() {
        practice_elapsedtime++;
        int remainingTime = 60 - practice_elapsedtime;
        if (remainingTime >= 0) {
            ui->practice_timerlabel->setText(QString("%1").arg(remainingTime));
        } else {
            practice_gameTimer->stop();
            onPracticeTypingFinished();
        }
    });


    connect(ui->typed, &QTextEdit::textChanged, this, &SpeedPaws::validateTypedText);
    connect(ui->practice_typed, &QTextEdit::textChanged, this, &SpeedPaws::validatePracticeTypedText);
    // connect(ui->new_race, &QPushButton::clicked, this, &SpeedPaws::on_replay_clicked);
    // connect(ui->back, &QPushButton::clicked, this, &SpeedPaws::on_back_clicked);

    //connect(ui->connect_button, &QPushButton::clicked, this, &SpeedPaws::on_connect_button_clicked);
    connect(webSocket, &QWebSocket::connected, this, &SpeedPaws::onConnected);
    connect(webSocket, &QWebSocket::textMessageReceived, this, &SpeedPaws::onTextMessageReceived);
    connect(webSocket, &QWebSocket::disconnected, this, &SpeedPaws::onDisconnected);

}

SpeedPaws::~SpeedPaws()
{
    delete ui;
}

void SpeedPaws::on_startbutton_clicked()
{ // this function is if the practice button is clicked


    ModeSelectionDialog modeDialog(this);
    QString selectedMode;
    connect(&modeDialog, &ModeSelectionDialog::modeSelected, [&](const QString &mode) {
        selectedMode = mode;
    });

    if (modeDialog.exec() == QDialog::Accepted) {
        ui->stackedWidget->setCurrentIndex(1);
        startPracticeMode(selectedMode);
    }
}

void SpeedPaws::startPracticeMode(const QString &mode) {
    ui->practice_stats->setVisible(false);
    ui->practice_replay->setVisible(false);
    QString filePath;
    if (mode == "Easy") {
        filePath = ":/paragraphs/easy.txt";
    } else if (mode == "Medium") {
        filePath = ":/paragraphs/medium.txt";
    } else if (mode == "Hard") {
        filePath = ":/paragraphs/hard.txt";
    }

    practice_targettext = getRandomParagraph(filePath);
    if (practice_targettext.isEmpty()) {
        if (mode == "Easy") {
            practice_targettext = "NUST campus cats love lounging under the warm sun near the library.";
        } else if (mode == "Medium") {
            practice_targettext = "The friendly cats at NUST are known to roam freely, bringing joy to students during their study breaks.";
        } else if (mode == "Hard") {
            practice_targettext = "Amid the bustling energy of NUST's academic life, the serene presence of campus cats provides a delightful juxtaposition, symbolizing peace and companionship.";
        }
    }
    ui->practice_typed->setReadOnly(true);
    ui->practice_text->setText(practice_targettext);
    update_practice_car_Position(0);
    ui->practice_typed->clear();
    ui->practice_timerlabel->setText("  ");
    ui->practice_result->clear();
    practice_countdown = 5;
    practice_elapsedtime = 0;
    ui->stackedWidget->setCurrentIndex(5);
    practice_countdownTimer->start(1000);
}

void SpeedPaws::onPracticeTypingFinished() {
    practice_currentIndex = 0;
    practice_gameTimer->stop();
    QString typed = ui->practice_typed->toPlainText();
    double wpm = calculateWPM(typed, practice_elapsedtime);
    double accuracy = calculateAccuracy(typed, practice_targettext);

    QString result = QString("WPM: %1\nAccuracy: %2")
                         .arg(QString::number(wpm, 'f', 2))
                         .arg(QString::number(accuracy, 'f', 2));

    ui->practice_result->setText(result);
    ui->practice_replay->setVisible(true);
    ui->practice_stats->setVisible(true);
    ui->practice_typed->setReadOnly(true);
    ui->practice_back->setVisible(true);
    ui->stackedWidget->setCurrentIndex(5);
}

void SpeedPaws::validatePracticeTypedText()
{
    QString practice_typed = ui->practice_typed->toPlainText();
    QString correct = practice_targettext.left(currentIndex + 1);

    if (!practice_typed.isEmpty() && practice_typed != correct) {
        // If the typed text doesn't match the correct text, truncate the typed text to the correct portion
        QString validText = practice_targettext.left(currentIndex);
        ui->practice_typed->blockSignals(true);  // Block signals to prevent triggering textChanged during modification
        ui->practice_typed->setPlainText(validText);  // Set valid typed text
        ui->practice_typed->blockSignals(false);  // Unblock signals
        ui->practice_typed->moveCursor(QTextCursor::End);  // Move cursor to the end
    }
    else if (practice_typed == correct) {
        currentIndex++;  // If the typed text matches the correct portion, move to the next character
        practice_playerProgress = calculateProgress(practice_typed, practice_targettext);  // Update player progress
        update_practice_car_Position(practice_playerProgress);
    }

    // Check if the entire target text is typed
    if (practice_typed == practice_targettext) {
        onPracticeTypingFinished();  // Call the function when typing is finished
    }
}

void SpeedPaws::update_practice_car_Position(int progress)
{
    // Ensure progress is within bounds (0 to 100)
    progress = qBound(0, progress, 100);  // Clamp progress between 0 and 100

    // Adjust the x position of the practice car based on the progress
    int trackWidth = ui->practice_racetrack->width();
    int carWidth = ui->practice_car->width();
    int x = practice_car_initial_x_position + (progress * (trackWidth - carWidth)) / 100;
    int y = practice_car_initial_y_position;

    ui->practice_car->move(x, y);
}


void SpeedPaws::validateTypedText()
{
    QString typed = ui->typed->toPlainText();
    QString correct = targettext.left(currentIndex + 1);

    if (!typed.isEmpty() && typed != correct) {
        QString validText = targettext.left(currentIndex);
        ui->typed->blockSignals(true);
        ui->typed->setPlainText(validText);
        ui->typed->blockSignals(false);
        ui->typed->moveCursor(QTextCursor::End);
    }
    else if (typed == correct) {
        currentIndex++;
        playerProgress = calculateProgress(typed, targettext);  // Update player progress
        update_player_1_car_Position(playerProgress);  // Reflect progress on UI
        sendProgressUpdate();  // Send progress update
    }


    if (typed == targettext) {
        onTypingFinished();
    }
}





void SpeedPaws::startGameTimer()
{
    gameTimer->start(1000);
}

void SpeedPaws::startPracticeGameTimer()
{
    practice_gameTimer->start(1000);
}

void SpeedPaws::onTypingFinished()
{
    currentIndex = 0;
    gameTimer->stop();
    QString typed = ui->typed->toPlainText();
    double wpm = calculateWPM(typed, elapsedtime);
    double accuracy = calculateAccuracy(typed, targettext);

    int finalProgress = playerProgress;
    int finalSpeed = static_cast<int>(wpm);

    // Send race finished message with updated stats
    sendRaceFinished(finalSpeed, finalProgress, elapsedtime);

    // QString result = QString("WPM: %1\nAccuracy: %2")
    //                      .arg(QString::number(wpm, 'f', 2))
    //                      .arg(QString::number(accuracy, 'f', 2));

    QString result = QString("Name: %1\nSpeed: %2 WPM\nProgress: %3%\nTime: %4 seconds")
                             .arg(playerName)
                             .arg(finalSpeed)
                             .arg(finalProgress)
                             .arg(elapsedtime);

    ui->my_result->setText(result);
    // ui->new_race->setVisible(true);
    // ui->stats->setVisible(true);
    ui->typed->setReadOnly(true);
    ui->stackedWidget->setCurrentIndex(4);
    // ui->back->setVisible(true);
}

// void SpeedPaws::on_replay_clicked()
// {
//     ui->stats->setVisible(false);
//     ui->new_race->setVisible(false);
//     ui->result->clear();
//     ui->typed->clear();
//     elapsedtime = 0;
//     countdown = 5;

//     ui->typed->clear();
//     ui->typing->setText(targettext);
//     ui->timerlabel->setText("");
//     ui->stackedWidget->setCurrentIndex(1);
//     update_player_1_car_Position(0);  // Reset car position
//     countdownTimer->start(1000);
//     ui->player_2_car->move(0, ui->player_2_car->y());

//     setOpponentName("");

//     playerProgress = 0;  // Reset progress
//     update_player_1_car_Position(0);  // Reset car position

// }

// void SpeedPaws::on_back_clicked()
// {
//     ui->typed->clear();
//     ui->stackedWidget->setCurrentIndex(0);
//     countdownTimer->stop();
//     gameTimer->stop();
//     // ui->stats->setVisible(false);
//     // ui->new_race->setVisible(false);
//     ui->typed->setReadOnly(true);

//     setOpponentName("");

//     playerProgress = 0;  // Reset progress
//     update_player_1_car_Position(0);  // Reset car position
// }

QString SpeedPaws::getRandomParagraph(const QString &fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return "";
    }

    QTextStream in(&file);
    QStringList paragraphs;
    while (!in.atEnd()) {
        paragraphs.append(in.readLine());
    }

    file.close();

    if (paragraphs.isEmpty()) return "";

    int randomIndex = QRandomGenerator::global()->bounded(paragraphs.size());
    return paragraphs[randomIndex];
}

void SpeedPaws::update_player_1_car_Position(int progress)
{
    // Ensure progress is within bounds (0 to 100)
    progress = qBound(0, progress, 100);  // Clamp progress between 0 and 100

    // Adjust the x position of the car based on the progress
    int trackWidth = ui->racetrack->width();
    int carWidth = ui->player_1_car->width();
    int x = player_1_initial_x_position + (progress * (trackWidth - carWidth)) / 100;
    int y = player_1_initial_y_position;

    ui->player_1_car->move(x, y);
}
int SpeedPaws::calculateProgress(const QString &typed, const QString &typing)
{
    return qMin((typed.length() * 100) / typing.length(), 100);
}

double SpeedPaws::calculateWPM(const QString &typed, int elapsedTimeInSeconds)
{
    if (elapsedTimeInSeconds == 0) return 0; // Avoid division by zero

    double wordsTyped = typed.length() / 5.0; // Approximate words typed
    double elapsedTimeInMinutes = elapsedTimeInSeconds / 60.0;

    return wordsTyped / elapsedTimeInMinutes;
}

double SpeedPaws::calculateAccuracy(const QString &typed, const QString &targettext)
{
    int correctChars = 0;
    int minLength = qMin(typed.length(), targettext.length());

    for (int i = 0; i < minLength; ++i) {
        if (typed[i] == targettext[i]) {
            correctChars++;
        }
    }

    return (static_cast<double>(correctChars) / minLength) * 100;
}

void SpeedPaws::setPlayerName(const QString &name) {
    playerName = name;
    ui->player_1->setText(playerName);
}

void SpeedPaws::setOpponentName(const QString &name) {
    opponentName = name;
    ui->player_2->setText(opponentName);
}


void SpeedPaws::on_connect_button_clicked() {

    setPlayerName(ui->usernameInput->text().trimmed());

    if (playerName.isEmpty()) {
        QMessageBox::warning(this, "Input Required", "Please enter a username before connecting.");
        return;
    }
    qDebug()<<playerName;

    QUrl url("ws://192.168.177.15:12345");  // WebSocket URL
    qDebug() << "Connecting to server at:" << url.toString();
    webSocket->open(url);
}

void SpeedPaws::onConnected() {
    qDebug() << "Connected to server!";

    sendJoinMessage();

    ui->stackedWidget->setCurrentIndex(2);  // Switch to the next UI screen
}



void SpeedPaws::onDisconnected() {
    qDebug() << "Disconnected from server.";
}

void SpeedPaws::sendJoinMessage() {
    if (playerName.isEmpty()) {
        QMessageBox::warning(this, "Input Required", "Please enter a username before connecting.");
        return;
    }

    QJsonObject joinObj;
    joinObj["type"] = "join";
    joinObj["playerName"] = playerName;  // Use playerName here
 qDebug()<<joinObj["playerName"];
    webSocket->sendTextMessage(QJsonDocument(joinObj).toJson(QJsonDocument::Compact));
}


void SpeedPaws::onTextMessageReceived(const QString &message) {
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject obj = doc.object();
    QString type = obj["type"].toString();

    if (type == "playerList") {
        updatePlayerList(obj["players"].toArray());
    } else if (type == "challengeRequest") {
        handleChallengeRequest(obj);
    } else if (type == "raceStart") {
        startRace(obj);
    } else if (type == "progressUpdate") {
        updateOpponentProgress(obj);
    } else if (type =="raceResult"){
        handleRaceResult(obj);
        qDebug()<<"Race Finished message received";
    } else if (type == "challengeRejected") {
        // Get the opponent's name from the message
        QString opponentName = obj["opponent"].toString();
        // Show a message box indicating the challenge was rejected
        QMessageBox::information(this, "Challenge Rejected", QString("%1 rejected your challenge.").arg(opponentName));
    }
}


void SpeedPaws::updatePlayerList(const QJsonArray &players) {
    ui->playerList->clear();
    QString currentPlayer = ui->usernameInput->text();  // Get current player's name

    for (const auto &player : players) {
        if (player.toString() != currentPlayer) {
            ui->playerList->addItem(player.toString());
        }
    }
}


void SpeedPaws::sendChallengeRequest(const QString &opponentName) {
    QJsonObject challengeObj;
    challengeObj["type"] = "challengeRequest";
    challengeObj["opponent"] = opponentName;

    webSocket->sendTextMessage(QJsonDocument(challengeObj).toJson(QJsonDocument::Compact));
}

void SpeedPaws::handleChallengeRequest(const QJsonObject &request) {
    QString challenger = request["challenger"].toString();
    int response = QMessageBox::question(this, "Challenge Request",
                                         QString("%1 has challenged you. Accept?").arg(challenger));

    // Create a response object to send back to the server
    QJsonObject responseObj;
    if (response == QMessageBox::Yes) {
        responseObj["type"] = "challengeAccepted";  // Player accepted the challenge
    } else {
        // Call the function to notify the server of the rejection
        sendChallengeRejected(challenger);
        return;  // Return early since we've already sent the rejection message
    }

    // Send the response (accept or reject) to the server
    responseObj["opponent"] = challenger;
    webSocket->sendTextMessage(QJsonDocument(responseObj).toJson(QJsonDocument::Compact));
}


void SpeedPaws::startRace(const QJsonObject &raceInfo) {
    targettext = raceInfo["paragraph"].toString();
    ui->opponent_result->setText("Waiting...");
    ui->typing->setText(targettext);
    ui->typed->clear();
    countdown = 5;
    elapsedtime = 0;
    ui->player_1_car->move(player_1_initial_x_position, player_1_initial_y_position);
    ui->player_2_car->move(player_2_initial_x_position, player_2_initial_y_position);
    ui->timerlabel->setText(" ");
    QString opponent = raceInfo["opponent"].toString();
    setOpponentName(opponent);  // Store opponent's name
    ui->new_race->setEnabled(false);
    ui->result_back->setEnabled(false);
    qDebug()<<"The opponent name is"<<opponent;

    ui->typed->setReadOnly(true);
    ui->countdownLabel->setText("Pawtyping starts in: 5");
    ui->stackedWidget->setCurrentIndex(3);

    countdownTimer->start(1000);

}



void SpeedPaws::sendProgressUpdate() {
    if (playerName.isEmpty()) {
        qWarning() << "Player name is empty!";
        return;
    }

    QJsonObject progressObj;
    progressObj["type"] = "progressUpdate";
    progressObj["player"] = playerName;  // Use playerName here
    progressObj["progress"] = playerProgress;

    webSocket->sendTextMessage(QJsonDocument(progressObj).toJson(QJsonDocument::Compact));
}



void SpeedPaws::updateOpponentProgress(const QJsonObject &progressInfo) {
    QString incomingPlayer = progressInfo["player"].toString();
    int opponentProgress = progressInfo["progress"].toInt();

    // Only update opponent's car if the progress is from them
    if (incomingPlayer == opponentName) {
        int trackWidth = ui->racetrack->width();
        int carWidth = ui->player_2_car->width();
        int newX = player_2_initial_x_position + (opponentProgress * (trackWidth - carWidth)) / 100;

        ui->player_2_car->move(newX, player_2_initial_y_position);
        qDebug() << "Opponent progress updated: " << opponentProgress;
    }
}

void SpeedPaws::on_multiplayer_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
}

void SpeedPaws::on_challenge_button_clicked()
{
    // Get the selected player from the player list
    QListWidgetItem *selectedItem = ui->playerList->currentItem();

    if (!selectedItem) {
        QMessageBox::warning(this, "No Player Selected", "Please select a player to challenge.");
        return;
    }

    QString opponentName = selectedItem->text();  // Get the name of the selected player
    qDebug() << "Challenging player:" << opponentName;

    // Send challenge request to the selected opponent
    sendChallengeRequest(opponentName);
}

void SpeedPaws::sendRaceFinished(int finalSpeed, int finalProgress, int timeTaken) {
    QJsonObject resultObj;
    resultObj["type"] = "raceFinished";
    resultObj["opponent"] = opponentName;
    resultObj["finalSpeed"] = finalSpeed;
    resultObj["finalProgress"] = finalProgress;
    resultObj["timeTaken"] = timeTaken;
    webSocket->sendTextMessage(QJsonDocument(resultObj).toJson(QJsonDocument::Compact));
}

void SpeedPaws::handleRaceResult(const QJsonObject &resultObj) {
    QString opponent = resultObj["opponent"].toString();
    int finalSpeed = resultObj["finalSpeed"].toInt();
    int finalProgress = resultObj["finalProgress"].toInt();
    int timeTaken = resultObj["timeTaken"].toInt() -1;

    QString resultText = QString("Name: %1\nSpeed: %2 WPM\nProgress: %3%\nTime: %4 seconds")
                             .arg(opponent)
                             .arg(finalSpeed)
                             .arg(finalProgress)
                             .arg(timeTaken);

    ui->opponent_result->setText(resultText);
    ui->new_race->setEnabled(true);
    ui->result_back->setEnabled(true);
    qDebug() << "Race result received for" << opponent;
    qDebug() << resultText;
}


void SpeedPaws::onConnectionError(QAbstractSocket::SocketError error)
{
    // Display a message box if the connection fails
    QMessageBox::critical(this, "Connection Error", "Failed to connect to the server. Please try again.");
}

void SpeedPaws::sendChallengeRejected(const QString &opponent) {
    // Create the JSON object to send to the server
    QJsonObject messageObj;
    messageObj["type"] = "challengeRejected";
    messageObj["opponent"] = opponent;  // Include the opponent's name

    // Convert the JSON object to a string
    QJsonDocument doc(messageObj);
    QString message = doc.toJson(QJsonDocument::Compact);

    // Send the message to the server
    if (webSocket->isValid()) {
        webSocket->sendTextMessage(message);
        qDebug() << "Challenge rejected message sent to server for opponent:" << opponent;
    } else {
        qDebug() << "WebSocket is not connected!";
    }
}

void SpeedPaws::on_practice_back_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->practice_typed->clear();
    ui->practice_stats->setVisible(false);
    ui->practice_replay->setVisible(false);
    ui->practice_typed->setReadOnly(true);
    practice_playerProgress = 0;  // Reset progress
    update_practice_car_Position(0);  // Reset car position
    ui->practice_text->setText(" ");
    ui->practice_result->clear();
    practice_countdownTimer->stop();
    practice_gameTimer->stop();
    practice_countdown = 5;
    practice_elapsedtime = 0;

}

void SpeedPaws::on_practice_replay_clicked()
{
    on_startbutton_clicked();
}

void SpeedPaws::on_new_race_pressed()
{
    ui->stackedWidget->setCurrentIndex(2);
    countdownTimer->stop();
    gameTimer->stop();
    countdown = 5;
    elapsedtime = 0;
    ui->player_1_car->move(player_1_initial_x_position, player_1_initial_y_position);
    ui->player_2_car->move(player_2_initial_x_position, player_2_initial_y_position);
    ui->timerlabel->setText(" ");
    ui->my_result->setText("");
    ui->opponent_result->setText("Waiting...");
}


void SpeedPaws::on_result_back_pressed()
{

    ui->stackedWidget->setCurrentIndex(2);
    countdownTimer->stop();
    gameTimer->stop();
    countdown = 5;
    elapsedtime = 0;
    ui->player_1_car->move(player_1_initial_x_position, player_1_initial_y_position);
    ui->player_2_car->move(player_2_initial_x_position, player_2_initial_y_position);
    ui->timerlabel->setText(" ");
    ui->my_result->setText("");
    ui->opponent_result->setText("Waiting...");

}


bool SpeedPaws::eventFilter(QObject *object, QEvent *event) {
    if (event->type() == QEvent::MouseButtonPress) {
        QPushButton *button = qobject_cast<QPushButton*>(object);

        if (button && !button->isEnabled()) {
            QMessageBox::information(this, "Button Disabled", "Wait for the opponent to finish the race.");
            return true;  // Stop further propagation
        }
    }
    // Pass to parent class for default behavior
    return QMainWindow::eventFilter(object, event);
}

void SpeedPaws::on_connect_back_clicked()
{
    int index = ui->stackedWidget->currentIndex()-1;
    ui->stackedWidget->setCurrentIndex(index);
}


void SpeedPaws::on_opponents_back_clicked()
{
    int index = ui->stackedWidget->currentIndex()-1;
    ui->stackedWidget->setCurrentIndex(index);
}

