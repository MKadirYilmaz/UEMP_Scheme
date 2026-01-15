Scheme: Multiplayer Strategy Framework

Scheme is a server-authoritative multiplayer card game built with Unreal Engine 5 (C++). The project demonstrates a modular, data-driven architecture designed to handle complex state synchronization, latency management, and secure gameplay logic within a Dedicated Server environment.

This project was developed using a custom source build of Unreal Engine 5 (Main Branch, Nov 1, 2025 commit) to leverage the latest engine features.

Engineering & Architecture

The core philosophy of the codebase is modularity and scalability. The system utilizes Unreal's Gameplay Framework heavily but extends it with custom handling for turn-based state machines and generic action execution.
1. Server-Authoritative Networking & Validation

To prevent cheating and ensure game integrity, the architecture follows a strict "Server-Authoritative" model. Clients are effectively "dumb terminals" regarding gameplay logic.

    Validation Layer: All critical actions (playing a card, challenging a bluff, exchanging gold) are validated server-side.

        Code Reference: ASchemeGameMode::ProcessActionRequest and ASchemeGameState::Server_AdvanceToNextPlayerTurn.

    Secure Replication: Sensitive data (such as the cards in a player's hand) uses DOREPLIFETIME_CONDITION with COND_OwnerOnly to ensure opponents cannot sniff packets to reveal hidden cards.

    Challenge Resolution: The logic for resolving bluffs is entirely encapsulated on the server (AutoResolveChallenge), which checks the private state of the accused player and replicates only the result (Success/Failure/Penalty) to clients.

2. Data-Driven Action System (Command Pattern)

Instead of hardcoding character abilities, the game uses a flexible UDataAsset pipeline. This allows for rapid iteration and the addition of new game mechanics without recompiling C++ code.

    UActionDataAsset: Defines the static properties of a move (Cost, Required Card, Counter-Card, Description).

    UBaseAction: An abstract class implementing the Command Pattern. Specific logic runs via ExecuteAction_Implementation. This decouples the definition of an action from its execution.

        Benefit: Designers can create new card variants in the Editor simply by linking different Action Logic classes to new Data Assets.

3. Finite State Machine (FSM) Game Loop

The game flow is managed by a strict FSM implemented within ASchemeGameMode.

    Phases: Idle → ActionReaction (Wait for challenges) → BlockReaction (Wait for counter-challenges) → Processing → TurnEnd.

    Timer Management: Server-side timers enforce decision windows. If a client disconnects or times out during a ReactionPhase, the server automatically resolves the state to keep the game flow uninterrupted.

4. Custom Notification Protocol

To synchronize complex UI events (like "Player X is challenging Player Y with Card Z") efficiently across the network, a custom struct-based protocol was implemented.

    FNotificationPacket: A lightweight struct sent via RPCs containing a USchemeNotification type, message text, and object references. This acts as a generic event bus, allowing the UI (Blueprint) to react dynamically to C++ backend state changes.

5. Interaction System

Implements IInteractableInterface to decouple the Player Controller from world objects.

    Uses raycasting from the camera perspective (UInteractionComponent) to detect focus.

    Supports different behaviors for Server (Game Logic) and Client (Visual Feedback/Highlighting) through interface implementation.

Code Structure Highlights

    Source/Scheme/Framework: Contains the core Game Loop (ASchemeGameMode) and State Management (ASchemeGameState).

    Source/Scheme/Gameplay/Data: The Data Asset definitions driving the modular gameplay.

    Source/Scheme/Player:

        ASchemePlayerController: Handles input and serves as the primary network interface for RPCs.

        ASchemePlayerState: Manages replicated variables like Gold, Elimination Status, and Player ID.

    Source/Scheme/Gameplay/Actors: Physical representations of game objects (Cards, Table) involving visual replication logic.

Game Overview

Scheme is a game of social deduction, bluffing, and strategy for 2-8 players.

    Objective: Be the last player with Influence (Face-down cards).

    Mechanic: Players can perform any action (Steal, Assassinate, Swap Cards) regardless of the cards they actually hold.

    The Twist: Any other player can Challenge an action. If the acting player cannot prove they hold the required character card, they lose a life (a card).

Build & Requirements

    Engine Version: Unreal Engine 5 (Source Build required for full compatibility, specifically Main Branch commit ~Nov 1, 2025).

    IDE: JetBrains Rider / Visual Studio 2022.

    Platform: Windows (Dedicated Server & Client).

Compilation

    Clone the repository.

    Right-click Scheme.uproject and select Generate Visual Studio project files.

    Build the Development Editor configuration.

    For multiplayer testing:

        Run with Net Mode: Play as Client (with a dedicated server enabled in Editor preferences) or use the packaging tool to build a standalone Server/Client.

Contributors

    [Muhammed Kadir Yilmaz]: Backend Architecture, Network Programming (C++), Game Systems Design.

    [Altay Bahadır Önsüz]: UI/UX Implementation, Visual Design

This project was developed as a Computer Engineering Graduation Project at Manisa Celal Bayar University.