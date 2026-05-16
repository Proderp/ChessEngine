#include "app.h"

App::App() : window(sf::VideoMode({ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }), "Chess"),
             renderer(window, game.playerSide),
             boardMetrics(renderer.getBoardMetrics()),
             historyViewportMetrics(renderer.getHistoryViewportMetrics())
{
    boardView = window.getDefaultView();

    uiView = window.getDefaultView();

    historyView = window.getDefaultView();
    historyView.setViewport(renderer.getHistoryViewport());
    historyView.setSize({ historyViewportMetrics.historyViewWidth, historyViewportMetrics.historyViewHeight });
    historyView.setCenter({ historyViewportMetrics.historyViewWidth / 2.f, historyViewportMetrics.historyViewHeight / 2.f });

    if (windowIcon.loadFromFile("assets/images/window-icon.png")) {
        window.setIcon(windowIcon);
    }

    handleResize(window.getSize().x, window.getSize().y);
}

void App::run() {
    while (window.isOpen()) {
        float deltaTime = static_cast<float>(frameClock.restart().asSeconds());

        Side previousTurn = game.getCurrentTurn();

        handleEvents();

        if (currentState == GameState::Playing) {
            if (game.getCurrentTurn() != previousTurn and game.getLastMove().has_value()) {
                scrollToBottom();
                startAnimation(game.getLastMove().value());

                previousTurn = game.getCurrentTurn();
            }
        }

        if (currentState == GameState::Playing or currentState == GameState::GameOver) {
            if (isAnimating) {
                handleAnimation(deltaTime);
            }
            else {
                if (currentState == GameState::Playing) {
                    game.updateAI();
                }
            }
        }

        if (currentState == GameState::Playing) {
            updateTimers(deltaTime);

            if (!isAnimating) {
                renderer.playerSide = game.playerSide;
            }

            if (game.getCurrentTurn() != previousTurn and game.getLastMove().has_value()) {
                scrollToBottom();
                startAnimation(game.getLastMove().value());
            }

            if (game.getWinner().has_value()) {
                currentState = GameState::GameOver;
            }
        }

        render();
    }
}

void App::handleAnimation(const float deltaTime) {
    currentAnimation.elapsedSeconds += deltaTime;
    
    float timePercentage = currentAnimation.elapsedSeconds / currentAnimation.durationSeconds;
    float totalLockDuration = animationLockdownDuration;

    if (timePercentage >= 1.0f) {
        timePercentage = 1.0f;
    }

    if (currentAnimation.elapsedSeconds >= totalLockDuration) {
        isAnimating = false;
        hasSecondaryAnimation = false;
    }

    currentAnimatedPosition.x = currentAnimation.startPixel.x + (currentAnimation.endPixel.x - currentAnimation.startPixel.x) * timePercentage;
    currentAnimatedPosition.y = currentAnimation.startPixel.y + (currentAnimation.endPixel.y - currentAnimation.startPixel.y) * timePercentage;

    if (hasSecondaryAnimation) {
        secondaryAnimatedPosition.x = secondaryAnimation.startPixel.x + (secondaryAnimation.endPixel.x - secondaryAnimation.startPixel.x) * timePercentage;
        secondaryAnimatedPosition.y = secondaryAnimation.startPixel.y + (secondaryAnimation.endPixel.y - secondaryAnimation.startPixel.y) * timePercentage;
    }
}

void App::handleEvents() {
    while (const std::optional<sf::Event> event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>())
            window.close();

        if (const auto* resizeEvent = event->getIf<sf::Event::Resized>()) {
            handleResize(resizeEvent->size.x, resizeEvent->size.y);
        }

        if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
            handleMouseClick(*mouseEvent);
        }

        if (const auto* scrollEvent = event->getIf<sf::Event::MouseWheelScrolled>()) {
            if (currentState == GameState::Playing) {
                handleScroll(*scrollEvent);
            }
        }

        if (const auto* mouseMovedEvent = event->getIf<sf::Event::MouseMoved>()) {
            if (currentState == GameState::Playing and isDragging) {
                currentMousePosition = window.mapPixelToCoords(mouseMovedEvent->position, boardView);
            }
        }

        if (const auto* mouseReleaseEvent = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseReleaseEvent->button == sf::Mouse::Button::Left and currentState == GameState::Playing) {
                handleMouseRelease(*mouseReleaseEvent);
            }
        }

        if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
            switch (keyPressed->scancode) {
            case sf::Keyboard::Scancode::Escape:
                handleEscapeButton();
                break;
            case sf::Keyboard::Scancode::F11:
                fullScreenToggle();
                break;
            }
        }
    }
}

void App::handleResize(const unsigned int windowWidth, const unsigned int windowHeight) {
    renderer.setInfo(windowWidth, windowHeight);
    setUIElements();
    recalibrateViews(windowWidth, windowHeight);
}

void App::setUIElements() {
    resizeResignationButton();

    resizeResignationConfirmationLayout();

    resizeLayoutButtons();

    resizePromoLayout();
    recalculateMoveHistorySize();

    resizeMainMenuLayout();
    
    resizePrePlayLayout();
    resizeSettingsLayout();

    resizeGameOverLayout();
}

void App::recalibrateViews(const unsigned int windowWidth, const unsigned int windowHeight) {
    const sf::Vector2f centerOfWindow = { static_cast<float>(windowWidth) / 2.f, static_cast<float>(windowHeight) / 2.f };
    const sf::Vector2f sizeOfWindow = static_cast<sf::Vector2f>(sf::Vector2u{windowWidth, windowHeight});

    boardView.setCenter(centerOfWindow);
    boardView.setSize(sizeOfWindow);

    uiView.setCenter(centerOfWindow);
    uiView.setSize(sizeOfWindow);

    historyView.setSize({ historyViewportMetrics.historyViewWidth, historyViewportMetrics.historyViewHeight });
    historyView.setCenter({ historyViewportMetrics.historyViewWidth / 2.f, historyViewportMetrics.historyViewHeight / 2.f });
    historyView.setViewport(renderer.getHistoryViewport());
}

void App::resizeResignationButton() {
    ui.resignationButton.position = { renderer.getWindowWidth() - boardMetrics.tileSize, renderer.getWindowHeight() - boardMetrics.offsetY};
    ui.resignationButton.size = { boardMetrics.tileSize, boardMetrics.tileSize };
    resizeButtonBounds(ui.resignationButton);
}

void App::resizeResignationConfirmationLayout() {
    ui.resignationConfirmationLayout.position = { historyViewportMetrics.historyViewStartX + historyViewportMetrics.historyViewWidth / 2.f, historyViewportMetrics.historyViewStartY + historyViewportMetrics.historyViewHeight / 2.f };
    ui.resignationConfirmationLayout.size = { historyViewportMetrics.historyViewWidth, historyViewportMetrics.historyViewHeight };
}
void App::resizeLayoutButtons() {
    ui.resignationConfirmationLayout.cancelButton.size = { historyViewportMetrics.historyViewWidth / 4.f, historyViewportMetrics.historyViewWidth / 8.f };
    ui.resignationConfirmationLayout.confirmButton.size = { historyViewportMetrics.historyViewWidth / 4.f, historyViewportMetrics.historyViewWidth / 8.f };

    const float xPositionBaseline = boardMetrics.offsetX + boardMetrics.boardSize + historyViewportMetrics.historyViewWidth / 2.f;
    const float yPositionBaseline = boardMetrics.offsetY + boardMetrics.boardSize / 2.f - boardMetrics.tileSize / 2.f;

    ui.resignationConfirmationLayout.cancelButton.position = { xPositionBaseline - historyViewportMetrics.historyViewWidth / 4.f, yPositionBaseline + boardMetrics.tileSize };
    ui.resignationConfirmationLayout.confirmButton.position = { xPositionBaseline + historyViewportMetrics.historyViewWidth / 4.f,  yPositionBaseline + boardMetrics.tileSize };

    resizeLayoutButtonBounds();
}

void App::resizeLayoutButtonBounds() {
    resizeButtonBounds(ui.resignationConfirmationLayout.cancelButton);
    resizeButtonBounds(ui.resignationConfirmationLayout.confirmButton);
}

void App::resizePromoLayout() {
    ui.promoMenuLayout.panelPosition = { boardMetrics.boardRightEdge + renderer.getMargin(), boardMetrics.boardTopEdge};
    ui.promoMenuLayout.panelSize = { boardMetrics.tileSize + (renderer.getPadding() * 2.f), (4.f * boardMetrics.tileSize) + (renderer.getPadding() * 2.f) + (3.f * renderer.getSpacing()) };

    for (std::size_t i = 0; i < ui.promoMenuLayout.slotRects.size(); i++) {
        sf::FloatRect rect{};
        rect.position = { ui.promoMenuLayout.panelPosition.x + renderer.getPadding(), ui.promoMenuLayout.panelPosition.y + renderer.getPadding() + (static_cast<float>(i) * (boardMetrics.tileSize + renderer.getSpacing()))};
        rect.size = { boardMetrics.tileSize, boardMetrics.tileSize };
        ui.promoMenuLayout.slotRects[i] = rect;
    }
}
void App::recalculateMoveHistorySize() {
    renderer.setRowDistance(boardMetrics.tileSize / 2.f);
    renderer.setMoveHistorySize(game.getMoveHistory().size() * renderer.getRowDistance() + renderer.getMargin());
}

void App::resizeMainMenuLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 3.f, boardMetrics.tileSize * 1.2f };
    ui.mainMenuLayout.playButton.size = buttonSize;
    ui.mainMenuLayout.settingsButton.size = buttonSize;
    ui.mainMenuLayout.quitButton.size = buttonSize;
    
    const sf::Vector2f centerOfWindow = { renderer.getWindowWidth() / 2.f, renderer.getWindowHeight() / 2.f };
    ui.mainMenuLayout.playButton.position = centerOfWindow;
    ui.mainMenuLayout.settingsButton.position = { centerOfWindow.x, centerOfWindow.y + buttonSize.y * 1.5f };
    ui.mainMenuLayout.quitButton.position = { centerOfWindow.x, centerOfWindow.y + buttonSize.y * 3.f };

    resizeButtonBounds(ui.mainMenuLayout.playButton);
    resizeButtonBounds(ui.mainMenuLayout.settingsButton);
    resizeButtonBounds(ui.mainMenuLayout.quitButton);
}

void App::resizeButtonBounds(Button& button) {
    const float halfButtonWidth = button.size.x / 2.f;
    const float halfButtonHeight = button.size.y / 2.f;
    
    const sf::Vector2f boundsPosition = { button.position.x - halfButtonWidth, button.position.y - halfButtonHeight };
    const sf::Vector2f boundsSize = button.size;

    button.bounds = sf::FloatRect(boundsPosition, boundsSize);
}

void App::resizePrePlayLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 1.25f, boardMetrics.tileSize * 1.25f};

    float currentXPosition = renderer.getWindowWidth() * 0.4f + buttonSize.x / 2.f;
    float currentYPosition = buttonSize.y;
    
    auto resizeSingleButton = [&](Button* button) {
        button->size = buttonSize;
        button->position = { currentXPosition, currentYPosition };
        resizeButtonBounds(*button);

        currentXPosition += buttonSize.x * 1.5f;
    };

    auto changePositions = [&]() {
        currentXPosition = renderer.getWindowWidth() * 0.4f + buttonSize.x / 2.f;
        currentYPosition += buttonSize.y * 1.5f;
    };

    for (Button* sideButton : ui.prePlayLayout.sideButtons) {
        resizeSingleButton(sideButton);
    }
    changePositions();

    for (Button* opponentButton : ui.prePlayLayout.opponentButtons) {
        resizeSingleButton(opponentButton);
    }
    changePositions();

    for (Button* timeControlButton : ui.prePlayLayout.timeControlButtons) {
        resizeSingleButton(timeControlButton);
    }
    changePositions();

    for (Button* incrementButton : ui.prePlayLayout.incrementButtons) {
        resizeSingleButton(incrementButton);
    }

    const sf::Vector2f startGameButtonSize = { boardMetrics.tileSize * 3.f, boardMetrics.tileSize * 1.25f };
    const sf::Vector2f startGameButtonPosition = { renderer.getWindowWidth() / 2.f, renderer.getWindowHeight() - startGameButtonSize.y };
    ui.prePlayLayout.startGameButton.size = startGameButtonSize;
    ui.prePlayLayout.startGameButton.position = startGameButtonPosition;
    resizeButtonBounds(ui.prePlayLayout.startGameButton);

    const sf::Vector2f backButtonSize = { boardMetrics.tileSize * 1.5f, boardMetrics.tileSize * 0.75f };
    ui.prePlayLayout.backButton.size = backButtonSize;
    ui.prePlayLayout.backButton.position = { renderer.getMargin() + backButtonSize.x / 2.f, renderer.getMargin() + backButtonSize.y / 2.f };
    resizeButtonBounds(ui.prePlayLayout.backButton);
}

void App::resizeSettingsLayout() {
    const sf::Vector2f buttonSize = { boardMetrics.tileSize * 1.25f, boardMetrics.tileSize * 1.25f };
    float xPosition = renderer.getWindowWidth() / 2.f + buttonSize.x / 2.f;
    float currentYPosition = buttonSize.y;

    for (Button* toggleButton : ui.settingsLayout.toggleButtons) {
        toggleButton->size = buttonSize;
        toggleButton->position = { xPosition, currentYPosition };
        resizeButtonBounds(*toggleButton);
        
        currentYPosition += buttonSize.y * 1.5f;
    }

    const sf::Vector2f backButtonSize = { boardMetrics.tileSize * 1.5f, boardMetrics.tileSize * 0.75f };
    ui.settingsLayout.backButton.size = backButtonSize;
    ui.settingsLayout.backButton.position = { renderer.getMargin() + backButtonSize.x / 2.f, renderer.getMargin() + backButtonSize.y / 2.f };
    resizeButtonBounds(ui.settingsLayout.backButton);
}

void App::resizeGameOverLayout() {
    const sf::Vector2f layoutSize = { renderer.getWindowWidth() * 0.4f, renderer.getWindowHeight() * 0.4f };
    ui.gameOverLayout.size = layoutSize;

    const sf::Vector2f centerOfWindow = { renderer.getWindowWidth() / 2.f, renderer.getWindowHeight() / 2.f };
    ui.gameOverLayout.position = centerOfWindow;

    const sf::Vector2f buttonSize = { layoutSize.x * 0.3f, layoutSize.y * 0.25f };
    ui.gameOverLayout.playAgainButton.size = buttonSize;
    ui.gameOverLayout.mainMenuButton.size = buttonSize;

    ui.gameOverLayout.playAgainButton.position = {
        centerOfWindow.x - (layoutSize.x * 0.25f),
        centerOfWindow.y + (layoutSize.y * 0.25f)
    };

    ui.gameOverLayout.mainMenuButton.position = {
        centerOfWindow.x + (layoutSize.x * 0.25f),
        centerOfWindow.y + (layoutSize.y * 0.25f)
    };

    resizeButtonBounds(ui.gameOverLayout.playAgainButton);
    resizeButtonBounds(ui.gameOverLayout.mainMenuButton);
}

void App::handleMouseClick(const sf::Event::MouseButtonPressed& mouseEvent) {
    if (mouseEvent.button == sf::Mouse::Button::Left) {
        handleLeftClick(mouseEvent);
    }

    if (mouseEvent.button == sf::Mouse::Button::Right and !game.getWinner().has_value()) {
        game.resetSelectedSquare();
    }
}

void App::handleLeftClick(const sf::Event::MouseButtonPressed& mouseEvent) {
    switch (currentState) {
    case GameState::MainMenu:
        handleLeftClickMainMenu(mouseEvent);
        break;
    case GameState::Settings:
        handleLeftClickSettings(mouseEvent);
        break;
    case GameState::PrePlay:
        handleLeftClickPrePlay(mouseEvent);
        break;
    case GameState::Playing:
        handleLeftClickPlaying(mouseEvent);
        break;
    case GameState::GameOver:
        handleLeftClickGameOver(mouseEvent);
        break;
    }
}

void App::handleLeftClickMainMenu(const sf::Event::MouseButtonPressed& mouseEvent) {
    const sf::Vector2f mousePosition = static_cast<sf::Vector2f>(mouseEvent.position);

    const bool userClickedPlayButton = ui.mainMenuLayout.playButton.bounds.contains(mousePosition);
    const bool userClickedSettingsButton = ui.mainMenuLayout.settingsButton.bounds.contains(mousePosition);
    const bool userClickedQuitButton = ui.mainMenuLayout.quitButton.bounds.contains(mousePosition);

    if (userClickedPlayButton) {
        currentState = GameState::PrePlay;
    }
    else if (userClickedSettingsButton) {
        currentState = GameState::Settings;
    }
    else if (userClickedQuitButton) {
        window.close();
    }
}

void App::handleLeftClickPrePlay(const sf::Event::MouseButtonPressed& mouseEvent) {
    const sf::Vector2f mousePosition = static_cast<sf::Vector2f>(mouseEvent.position);

    if (ui.prePlayLayout.startGameButton.bounds.contains(mousePosition)) {
        game = Game("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", playerChoices);
        renderer.playerSide = game.playerSide;
        currentState = GameState::Playing;
        frameClock.restart();
        return;
    }

    if (ui.prePlayLayout.backButton.bounds.contains(mousePosition)) {
        currentState = GameState::MainMenu;
    }

    for (int buttonIndex = 0; buttonIndex < ui.prePlayLayout.sideButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = ui.prePlayLayout.sideButtons[buttonIndex]->bounds;
        if (bounds.contains(mousePosition)) {
            switch (buttonIndex) {
            case 0:
                playerChoices.sideChoice = Side::White;
                break;
            case 1:
                playerChoices.sideChoice = Side::Black;
                break;
            case 2:
                playerChoices.sideChoice = Side::Random;
                break;
            }
            return;
        }
    }

    for (int buttonIndex = 0; buttonIndex < ui.prePlayLayout.opponentButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = ui.prePlayLayout.opponentButtons[buttonIndex]->bounds;
        if (bounds.contains(mousePosition)) {
            switch (buttonIndex) {
            case 0:
                playerChoices.opponentChoice = OpponentType::Engine;
                break;
            case 1:
                playerChoices.opponentChoice = OpponentType::Human;
                break;
            }
            return;
        }
    }

    for (int buttonIndex = 0; buttonIndex < ui.prePlayLayout.timeControlButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = ui.prePlayLayout.timeControlButtons[buttonIndex]->bounds;
        if (bounds.contains(mousePosition)) {
            switch (buttonIndex) {
            case 0:
                playerChoices.timeControlChoice = TimeControl::Bullet;
                break;
            case 1:
                playerChoices.timeControlChoice = TimeControl::Blitz;
                break;
            case 2:
                playerChoices.timeControlChoice = TimeControl::Rapid;
                break;
            }
            return;
        }
    }

    for (int buttonIndex = 0; buttonIndex < ui.prePlayLayout.incrementButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = ui.prePlayLayout.incrementButtons[buttonIndex]->bounds;
        if (bounds.contains(mousePosition)) {
            playerChoices.timeIncrement = timeIncrementChoices.at(buttonIndex);
            return;
        }
    }
}

void App::handleLeftClickSettings(const sf::Event::MouseButtonPressed& mouseEvent) {
    const sf::Vector2f mousePosition = static_cast<sf::Vector2f>(mouseEvent.position);

    if (ui.settingsLayout.backButton.bounds.contains(mousePosition)) {
        currentState = GameState::MainMenu;
        return;
    }

    if (ui.settingsLayout.toggleButtons[0]->bounds.contains(mousePosition)) {
        fullScreenToggle();
        return;
    }

    if (ui.settingsLayout.toggleButtons[1]->bounds.contains(mousePosition)) {
        settings.showLegalMoves = !settings.showLegalMoves;
        return;
    }

    if (ui.settingsLayout.toggleButtons[2]->bounds.contains(mousePosition)) {
        settings.autoPromoteToQueen = !settings.autoPromoteToQueen;
        return;
    }
}

void App::handleLeftClickPlaying(const sf::Event::MouseButtonPressed& mouseEvent) {
    if (game.getWinner().has_value()) {
        return;
    }

    sf::Vector2i mousePosition = mouseEvent.position;
    const sf::Vector2f uiPos = window.mapPixelToCoords(mousePosition, uiView);

    if (game.getIsConfirmingResignation()) {
        handleResignation(uiPos);
        return;
    }

    const bool userClickedResignationButton = ui.resignationButton.bounds.contains(uiPos);

    if (userClickedResignationButton) {
        game.resetSelectedSquare();
        game.setIsConfirmingResignation(true);
    }
    else if (game.isPromoPending()) {
        handlePromotion(uiPos);
    }
    else {
        handleClickOnBoard(mousePosition);
    }
}

void App::handleLeftClickGameOver(const sf::Event::MouseButtonPressed& mouseEvent) {
    const sf::Vector2f mousePosition = static_cast<sf::Vector2f>(mouseEvent.position);

    if (ui.gameOverLayout.mainMenuButton.bounds.contains(mousePosition)) {
        currentState = GameState::MainMenu;
        return;
    } 

    if (ui.gameOverLayout.playAgainButton.bounds.contains(mousePosition)) {
        currentState = GameState::PrePlay;
        return;
    }
}

void App::handleClickOnBoard(const sf::Vector2i& mousePosition) {
    if (isAnimating) {
        return;
    }

    const sf::Vector2f worldPos = window.mapPixelToCoords(mousePosition, boardView);

    int clickedRank = static_cast<int>(floor((worldPos.y - boardMetrics.offsetY + boardMetrics.tileSize / 2.f) / boardMetrics.tileSize));
    int clickedFile = static_cast<int>(floor((worldPos.x - boardMetrics.offsetX + boardMetrics.tileSize / 2.f) / boardMetrics.tileSize));

    if (game.playerSide == Side::Black) {
        clickedRank = 7 - clickedRank;
        clickedFile = 7 - clickedFile;
    }

    if (clickedRank == game.getSelectedRank() and clickedFile == game.getSelectedFile()) {
        isDragging = true;
        currentMousePosition = worldPos;
        return;
    }

    wasJustDragged = false;
    game.processBoardClick(clickedRank, clickedFile);

    if (settings.autoPromoteToQueen and game.isPromoPending()) {
        game.handlePendingPromo(PieceType::Queen);
    }

    renderer.updateGraveyardCounts(game.getWhiteGraveyard(), game.getBlackGraveyard());

    if (game.getSelectedRank() != INVALID_COORD) {
        isDragging = true;
        currentMousePosition = worldPos;
    }
}

void App::startAnimation(const Move& lastMove) {
    isAnimating = true;

    int drawnToRank = (renderer.playerSide == Side::Black) ? 7 - lastMove.toRank : lastMove.toRank;
    int drawnToFile = (renderer.playerSide == Side::Black) ? 7 - lastMove.toFile : lastMove.toFile;

    currentAnimation.endPixel = {
        boardMetrics.offsetX + (drawnToFile * boardMetrics.tileSize),
        boardMetrics.offsetY + (drawnToRank * boardMetrics.tileSize)
    };

    if (wasJustDragged) {
        currentAnimation.startPixel = currentMousePosition;
        wasJustDragged = false;
    }
    else {
        int drawnFromRank = (renderer.playerSide == Side::Black) ? 7 - lastMove.fromRank : lastMove.fromRank;
        int drawnFromFile = (renderer.playerSide == Side::Black) ? 7 - lastMove.fromFile : lastMove.fromFile;

        currentAnimation.startPixel = {
            boardMetrics.offsetX + (drawnFromFile * boardMetrics.tileSize),
            boardMetrics.offsetY + (drawnFromRank * boardMetrics.tileSize)
        };
    }

    currentAnimation.elapsedSeconds = 0.f;
    currentAnimation.toRank = lastMove.toRank;
    currentAnimation.toFile = lastMove.toFile;

    currentAnimation.movingPiece = game.getBoard()[lastMove.toRank][lastMove.toFile].piece.value();
    currentAnimatedPosition = currentAnimation.startPixel;

    hasSecondaryAnimation = false;

    if (currentAnimation.movingPiece.type == PieceType::King and std::abs(lastMove.toFile - lastMove.fromFile) == 2) {
        hasSecondaryAnimation = true;

        int rookToRank = lastMove.toRank;
        int rookToFile = (lastMove.toFile > lastMove.fromFile) ? 5 : 3;

        int rookFromRank = lastMove.toRank;
        int rookFromFile = (lastMove.toFile > lastMove.fromFile) ? 7 : 0;

        secondaryAnimation.toRank = rookToRank;
        secondaryAnimation.toFile = rookToFile;
        secondaryAnimation.movingPiece = game.getBoard()[rookToRank][rookToFile].piece.value();

        int drawnRookFromRank = (renderer.playerSide == Side::Black) ? 7 - rookFromRank : rookFromRank;
        int drawnRookFromFile = (renderer.playerSide == Side::Black) ? 7 - rookFromFile : rookFromFile;
        int drawnRookToRank = (renderer.playerSide == Side::Black) ? 7 - rookToRank : rookToRank;
        int drawnRookToFile = (renderer.playerSide == Side::Black) ? 7 - rookToFile : rookToFile;

        secondaryAnimation.startPixel = {
            boardMetrics.offsetX + (drawnRookFromFile * boardMetrics.tileSize),
            boardMetrics.offsetY + (drawnRookFromRank * boardMetrics.tileSize)
        };
        secondaryAnimation.endPixel = {
            boardMetrics.offsetX + (drawnRookToFile * boardMetrics.tileSize),
            boardMetrics.offsetY + (drawnRookToRank * boardMetrics.tileSize)
        };

        secondaryAnimatedPosition = secondaryAnimation.startPixel;
    }
}

void App::handleResignation(const sf::Vector2f uiPos) {
    const bool userConfirmedResignation = ui.resignationConfirmationLayout.confirmButton.bounds.contains(uiPos);
    const bool userCancelledResignation = ui.resignationConfirmationLayout.cancelButton.bounds.contains(uiPos);

    if (userConfirmedResignation) {
        const Side winner = (game.playerSide == Side::White) ? Side::Black : Side::White;
        game.setWinner(winner);
        game.setGameOverType(GameOverType::Resignation);
        game.setIsConfirmingResignation(false);
    }
    else if (userCancelledResignation) {
        game.setIsConfirmingResignation(false);
    }
}
void App::handlePromotion(const sf::Vector2f uiPos) {
    const std::optional<PieceType> chosenPiece = promoMenuPick(uiPos, game.getPromoMenuSide());

    if (chosenPiece.has_value()) {
        game.handlePendingPromo(chosenPiece.value());
    }
}
std::optional<PieceType> App::promoMenuPick(const sf::Vector2f& uiPos, const std::optional<Side>& promotionSide) const {
    if (!promotionSide.has_value())
        return std::nullopt;

    for (std::size_t i = 0; i < ui.promoMenuLayout.slotRects.size(); i++) {
        if (ui.promoMenuLayout.slotRects[i].contains(uiPos))
            return promotionPieceTypes.at(i);
    }

    return std::nullopt;
}

void App::handleScroll(const sf::Event::MouseWheelScrolled& scrollEvent) {
    if (scrollEvent.wheel == sf::Mouse::Wheel::Vertical) {
        const float scrollSpeed = boardMetrics.tileSize / 2.f;
        const float scrollDirection = scrollEvent.delta * -1;

        historyView.move({ 0.f, scrollSpeed * scrollDirection });

        limitScroll();
    }
}
void App::limitScroll() {
    const float halfViewHeight = historyView.getSize().y / 2.f;

    const float minCenterY = halfViewHeight;
    const float maxCenterY = std::max(minCenterY, renderer.getMoveHistorySize() - halfViewHeight + renderer.getMargin());

    sf::Vector2f center = historyView.getCenter();

    if (center.y < minCenterY) center.y = minCenterY;
    if (center.y > maxCenterY) center.y = maxCenterY;

    historyView.setCenter(center);
}
void App::scrollToBottom() {
    recalculateMoveHistorySize();
    historyView.move({ 0.f, 99999.f });
    limitScroll();
}

void App::handleMouseRelease(const sf::Event::MouseButtonReleased& mouseReleased) {
    if (!isDragging)
        return;

    isDragging = false;

    const sf::Vector2f worldPos = window.mapPixelToCoords(mouseReleased.position, boardView);

    int releaseRank = static_cast<int>(floor((worldPos.y - boardMetrics.offsetY + boardMetrics.tileSize / 2.f) / boardMetrics.tileSize));
    int releaseFile = static_cast<int>(floor((worldPos.x - boardMetrics.offsetX + boardMetrics.tileSize / 2.f) / boardMetrics.tileSize));

    if (game.playerSide == Side::Black) {
        releaseRank = 7 - releaseRank;
        releaseFile = 7 - releaseFile;
    }

    if (releaseRank == game.getSelectedRank() and releaseFile == game.getSelectedFile()) {
        return;
    }

    wasJustDragged = true;
    game.processBoardClick(releaseRank, releaseFile);

    if (settings.autoPromoteToQueen and game.isPromoPending()) {
        game.handlePendingPromo(PieceType::Queen);
    }

    renderer.updateGraveyardCounts(game.getWhiteGraveyard(), game.getBlackGraveyard());
}

void App::handleEscapeButton() {
    if (settings.fullScreen) {
        settings.fullScreen = !settings.fullScreen;
        window.create(sf::VideoMode({ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }), "Chess", sf::Style::Default);
        handleResize(window.getSize().x, window.getSize().y);
    }
}

void App::fullScreenToggle() {
    settings.fullScreen = !settings.fullScreen;
    if (settings.fullScreen) {
        window.create(sf::VideoMode::getFullscreenModes().front(), "Chess", sf::State::Fullscreen);
    }
    else {
        window.create(sf::VideoMode({ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }), "Chess", sf::Style::Default);
    }

    handleResize(window.getSize().x, window.getSize().y);
}

void App::updateTimers(const float deltaTime) {
    game.updateTimers(deltaTime);
}

void App::render() {
    window.clear(sf::Color(152, 123, 96));

    switch (currentState) {
    case GameState::MainMenu:
        renderMainMenu();
        break;
    case GameState::Settings:
        renderSettings();
        break;
    case GameState::PrePlay:
        renderPrePlay();
        break;
    case GameState::Playing:
        renderPlayingState();
        break;
    case GameState::GameOver:
        renderGameOverState();
        break;
    }

    window.display();
}

void App::renderMainMenu() {
    window.setView(uiView);
    renderer.drawMainMenu(ui.mainMenuLayout);
}

void App::renderPrePlay() {
    window.setView(uiView);
    renderer.drawPrePlayPage(ui.prePlayLayout, playerChoices);
}

void App::renderSettings() {
    window.setView(uiView);
    renderer.drawSettingsPage(ui.settingsLayout, settings);
}

void App::renderPlayingState() {
    renderBoardView();

    if (game.getIsConfirmingResignation()) {
        window.setView(uiView);
        renderer.drawResignationConfirmation(ui.resignationConfirmationLayout);
    }
    else {
        renderHistoryView();
    }

    renderUIView();
}
void App::renderBoardView() {
    window.setView(boardView);
    renderer.drawBoard(game.getBoard(), game.getSelectedRank(), game.getSelectedFile(), game.getLastMove());

    renderer.drawPieces(
        game.getBoard(),
        isDragging, currentMousePosition,
        game.getSelectedRank(), game.getSelectedFile(),
        isAnimating,
        currentAnimation.toRank, currentAnimation.toFile,
        currentAnimatedPosition,
        hasSecondaryAnimation,
        secondaryAnimation.toRank, secondaryAnimation.toFile,
        secondaryAnimatedPosition
    );

    if (settings.showLegalMoves) {
        renderer.drawLegalMoves(game.getSelectedPieceMoves());
    }
}
void App::renderHistoryView() {
    sf::RectangleShape testRect({ historyViewportMetrics.historyViewWidth, 5'000.f });
    testRect.setFillColor(sf::Color(100, 149, 237, 100));

    window.setView(historyView);
    window.draw(testRect);
    renderer.drawMoveHistory(game.getMoveHistory());
}
void App::renderUIView() {
    window.setView(uiView);
    renderer.drawTimers(game.getWhiteTime(), game.getBlackTime());
    renderer.drawResignationButton(ui.resignationButton);
    renderer.drawRanksAndFiles(game.getCurrentTurn());
    renderer.drawPromoMenu(game.getPromoMenuSide());
    renderer.drawGraveyards(game.getBoardMaterial());
}

void App::renderGameOverState() {
    renderPlayingState();

    window.setView(uiView);
    renderer.drawGameOverLayout(ui.gameOverLayout, game.getGameOverType(), game.getWinner());
}