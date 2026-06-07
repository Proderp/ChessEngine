#include "app.hpp"

App::App() : 
    window(sf::VideoMode({ DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT }), "Chess"),
    windowSize(sf::Vector2f(window.getSize())),
    ui(window, windowSize),
    renderer(ui.getBoardMetrics(), window, windowSize, game.playerSide)
{
    window.setFramerateLimit(60); 
    
    if (windowIcon.loadFromFile("assets/window/window-icon.png")) {
        window.setIcon(windowIcon);
    }

    handleResize();
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
            handleResize();
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
                currentMousePosition = window.mapPixelToCoords(mouseMovedEvent->position, ui.getBoardView());
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

void App::handleResize() {
    windowSize = static_cast<sf::Vector2f>(window.getSize());
    ui.updateUI();
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

    const MainMenuLayout& mainMenu = ui.getMainMenuLayout();
    const bool userClickedPlayButton = mainMenu.playButton.bounds.contains(mousePosition);
    const bool userClickedSettingsButton = mainMenu.settingsButton.bounds.contains(mousePosition);
    const bool userClickedQuitButton = mainMenu.quitButton.bounds.contains(mousePosition);

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

    const PrePlayLayout& prePlay = ui.getPrePlayLayout();

    if (prePlay.startGameButton.bounds.contains(mousePosition)) {
        // starting: 
        // KNBvK: 8/8/8/4k3/8/8/8/5KBN w - - 0 1
        game = Game("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1", playerChoices);
        renderer.playerSide = game.playerSide;
        currentState = GameState::Playing;
        frameClock.restart();
        return;
    }

    if (prePlay.backButton.bounds.contains(mousePosition)) {
        currentState = GameState::MainMenu;
    }

    for (int buttonIndex = 0; buttonIndex < prePlay.sideButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = prePlay.sideButtons[buttonIndex]->bounds;
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

    for (int buttonIndex = 0; buttonIndex < prePlay.opponentButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = prePlay.opponentButtons[buttonIndex]->bounds;
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

    for (int buttonIndex = 0; buttonIndex < prePlay.timeControlButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = prePlay.timeControlButtons[buttonIndex]->bounds;
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

    for (int buttonIndex = 0; buttonIndex < prePlay.incrementButtons.size(); buttonIndex++) {
        const sf::FloatRect& bounds = prePlay.incrementButtons[buttonIndex]->bounds;
        if (bounds.contains(mousePosition)) {
            playerChoices.timeIncrement = timeIncrementChoices.at(buttonIndex);
            return;
        }
    }
}

void App::handleLeftClickSettings(const sf::Event::MouseButtonPressed& mouseEvent) {
    const sf::Vector2f mousePosition = static_cast<sf::Vector2f>(mouseEvent.position);
    const SettingsLayout& settingsLayout = ui.getSettingsLayout();

    if (settingsLayout.backButton.bounds.contains(mousePosition)) {
        currentState = GameState::MainMenu;
        return;
    }

    if (settingsLayout.toggleButtons[0]->bounds.contains(mousePosition)) {
        fullScreenToggle();
        return;
    }

    if (settingsLayout.toggleButtons[1]->bounds.contains(mousePosition)) {
        settings.showLegalMoves = !settings.showLegalMoves;
        return;
    }

    if (settingsLayout.toggleButtons[2]->bounds.contains(mousePosition)) {
        settings.autoPromoteToQueen = !settings.autoPromoteToQueen;
        return;
    }
}

void App::handleLeftClickPlaying(const sf::Event::MouseButtonPressed& mouseEvent) {
    if (game.getWinner().has_value()) {
        return;
    }

    sf::Vector2i mousePosition = mouseEvent.position;
    const sf::Vector2f uiPos = window.mapPixelToCoords(mousePosition, ui.getUIView());

    if (game.getIsConfirmingResignation()) {
        handleResignation(uiPos);
        return;
    }

    const bool userClickedResignationButton = ui.getResignationButton().bounds.contains(uiPos);

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
    const GameOverLayout& gameOverLayout = ui.getGameOverLayout();

    if (gameOverLayout.mainMenuButton.bounds.contains(mousePosition)) {
        currentState = GameState::MainMenu;
        return;
    } 

    if (gameOverLayout.playAgainButton.bounds.contains(mousePosition)) {
        currentState = GameState::PrePlay;
        return;
    }
}

void App::handleClickOnBoard(const sf::Vector2i& mousePosition) {
    if (isAnimating) {
        return;
    }

    const sf::Vector2f worldPos = window.mapPixelToCoords(mousePosition, ui.getBoardView());

    const BoardMetrics& boardMetrics = ui.getBoardMetrics();
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

    const BoardMetrics& boardMetrics = ui.getBoardMetrics();
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
    const bool userConfirmedResignation = ui.getResignationConfirmationLayout().confirmButton.bounds.contains(uiPos);
    const bool userCancelledResignation = ui.getResignationConfirmationLayout().cancelButton.bounds.contains(uiPos);

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
std::optional<PieceType> App::promoMenuPick(const sf::Vector2f& uiPos, const std::optional<Side>& promotionSide) {
    if (!promotionSide.has_value())
        return std::nullopt;

    const PromoMenuLayout& promoMenu = ui.getPromoMenuLayout();
    for (std::size_t i = 0; i < promoMenu.slotRects.size(); i++) {
        if (promoMenu.slotRects[i].contains(uiPos))
            return promotionPieceTypes.at(i);
    }

    return std::nullopt;
}

void App::handleScroll(const sf::Event::MouseWheelScrolled& scrollEvent) {
    if (scrollEvent.wheel == sf::Mouse::Wheel::Vertical) {
        const float scrollSpeed = ui.getBoardMetrics().tileSize / 2.f;
        const float scrollDirection = scrollEvent.delta * -1;

        ui.moveHistoryView({ 0.f, scrollSpeed * scrollDirection });

        limitScroll();
    }
}
void App::limitScroll() {
    const float halfViewHeight = ui.getHistoryView().getSize().y / 2.f;

    const float minCenterY = halfViewHeight;
    const float maxCenterY = std::max(minCenterY, renderer.getMoveHistorySize() - halfViewHeight + ui.getMargin());

    sf::Vector2f center = ui.getHistoryView().getCenter();

    if (center.y < minCenterY) center.y = minCenterY;
    if (center.y > maxCenterY) center.y = maxCenterY;

    ui.setHistoryViewCenter(center);
}
void App::scrollToBottom() {
    recalculateMoveHistorySize();
    ui.moveHistoryView({ 0.f, 99999.f });
    limitScroll();
}

void App::handleMouseRelease(const sf::Event::MouseButtonReleased& mouseReleased) {
    if (!isDragging)
        return;

    isDragging = false;

    const sf::Vector2f worldPos = window.mapPixelToCoords(mouseReleased.position, ui.getBoardView());
    
    const BoardMetrics& boardMetrics = ui.getBoardMetrics();
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
        handleResize();
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

    handleResize();
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
    ui.setUIView();
    renderer.drawMainMenu(ui.getMainMenuLayout());
}

void App::renderPrePlay() {
    ui.setUIView();
    renderer.drawPrePlayPage(ui.getPrePlayLayout(), playerChoices);
}

void App::renderSettings() {
    ui.setUIView();
    renderer.drawSettingsPage(ui.getSettingsLayout(), settings);
}

void App::renderPlayingState() {
    renderBoardView();

    if (game.getIsConfirmingResignation()) {
        ui.setUIView();
        renderer.drawResignationConfirmation(ui.getResignationConfirmationLayout(), ui.getHistoryViewportMetrics());
    }
    else {
        renderHistoryView();
    }

    renderUIView();
}
void App::renderBoardView() {
    ui.setBoardView();
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
    sf::RectangleShape testRect({ ui.getHistoryViewportMetrics().historyViewWidth, 5'000.f });
    testRect.setFillColor(sf::Color(100, 149, 237, 100));

    ui.setHistoryView();
    window.draw(testRect);
    renderer.drawMoveHistory(game.getMoveHistory(), ui.getHistoryViewportMetrics());
}
void App::renderUIView() {
    ui.setUIView();
    renderer.drawTimers(game.getWhiteTime(), game.getBlackTime());
    renderer.drawResignationButton(ui.getResignationButton());
    renderer.drawRanksAndFiles(game.getCurrentTurn());
    renderer.drawPromoMenu(game.getPromoMenuSide());
    renderer.drawGraveyards(game.getBoardMaterial());
}

void App::renderGameOverState() {
    renderPlayingState();

    ui.setUIView();
    renderer.drawGameOverLayout(ui.getGameOverLayout(), game.getGameOverType(), game.getWinner());
}