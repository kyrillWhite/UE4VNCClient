#include "Control.h"
#include "VNCClient.h"

Control::Control() :
    playerController(nullptr),
    mesh(nullptr),
    keyboardWidgetComponent(nullptr),
    inputEnabled(false),
    widgetInteractionComponent(nullptr),
    lastCursorPosition(FVector(0)),
    mouseState({ 0, 0, 0, 0, 0, 0, 0, 0, 0 })
{
}

Control::~Control()
{
}

FVector Control::GetCameraLookProjectionOnPlane(bool& isOutOfBounds)
{
    if (!playerController || !mesh) return FVector();

    UWorld* world = playerController->GetWorld();
    int viewportSizeX, viewportSizeY;
    playerController->GetViewportSize(viewportSizeX, viewportSizeY);
    FVector vectorStart, vectorDirection;
    playerController->DeprojectScreenPositionToWorld(viewportSizeX * 0.5f, viewportSizeY * 0.5f, vectorStart, vectorDirection);
    const FVector vectorEnd = vectorStart + vectorDirection * 99999.9f;

    FTransform meshTransform = mesh->GetComponentTransform();

    UStaticMesh* staticMesh = mesh->GetStaticMesh();
    const FStaticMeshLODResources& LODResource = staticMesh->RenderData->LODResources[0];
    TArray<FVector> vertices;

    const FPositionVertexBuffer* vertexBuffer = &staticMesh->RenderData->LODResources[0].VertexBuffers.PositionVertexBuffer;
    const int32 vertexCount = vertexBuffer->GetNumVertices();
    for (int32 i = 0; i < vertexCount; i++) {
        vertices.Add(meshTransform.TransformPosition(vertexBuffer->VertexPosition(i)));
    }

    FPlane plane = FPlane(vertices[0], vertices[1], vertices[2]);
    FVector intersectionPoint = FMath::LinePlaneIntersection(vectorStart, vectorEnd, plane);
    FlushPersistentDebugLines(world);
    DrawDebugPoint(world, intersectionPoint, 10, FColor::Red, true, 0, 0);
    intersectionPoint = meshTransform.InverseTransformPosition(intersectionPoint);
    FVector normalizedPoint = (intersectionPoint + 50.f) / 100.f;

    isOutOfBounds = normalizedPoint.X < 0 || normalizedPoint.X >= 1 ||
        normalizedPoint.Y < 0 || normalizedPoint.Y >= 1;

    // from 0 to 1
    return normalizedPoint;
}

FVector Control::GetPositionFromWidgetInteractoin(bool& isOutOfBounds, bool& isWidget)
{
    isOutOfBounds = true;
    isWidget = false;
    if (!widgetInteractionComponent) {
        return GetCameraLookProjectionOnPlane(isOutOfBounds);
    }

    auto widgetComponent = widgetInteractionComponent->GetHoveredWidgetComponent();
    if (!widgetComponent) {
        return lastCursorPosition;
    }
    isWidget = true;

    auto userWidget = widgetComponent->GetUserWidgetObject();
    auto userWidgetName = userWidget->GetName();
    FString screenName = "Screen";
    if (!userWidgetName.Contains(screenName)) {
        return lastCursorPosition;
    }

    auto hitResult = widgetInteractionComponent->Get2DHitLocation();
    auto widgetSize = userWidget->GetDesiredSize();
    FVector cursorPosition = { hitResult.Y / widgetSize.Y, 1.f - hitResult.X / widgetSize.X, 0 };
    lastCursorPosition = cursorPosition;
    isOutOfBounds = false;
    return cursorPosition;
}

bool Control::IsInputEnabled(FKey key)
{
    if (!playerController || !mesh) {
        return false;
    }

    if (!inputEnabled) {
        if (key == EKeys::Invalid || !key.IsMouseButton()) {
            return false;
        }

        bool isOutOfBounds, isWidget;

        FVector localPosition = GetPositionFromWidgetInteractoin(isOutOfBounds, isWidget);

        if (!isOutOfBounds) {
            playerController->SetIgnoreMoveInput(true);
            if (keyboardWidgetComponent) {
                keyboardWidgetComponent->SetVisibility(true);
            }
            inputEnabled = true;
        }
        return false;
    }

    return true;
}

bool Control::HandleMouseEvent(FKey key, bool isPressed, SMouseEvent& _event, uint16 width, uint16 height)
{
    bool isOutOfBounds, isWidget;
    FVector localPosition = GetPositionFromWidgetInteractoin(isOutOfBounds, isWidget);
    if (isOutOfBounds) {
        if (key != EKeys::Invalid && !isWidget) {
            playerController->ResetIgnoreMoveInput();
            if (keyboardWidgetComponent) {
                keyboardWidgetComponent->SetVisibility(false);
            }
            inputEnabled = false;
        }
        return false;
    }

    if (key == EKeys::LeftMouseButton) {
        mouseState.leftPressed = isPressed;
    }
    else if (key == EKeys::MiddleMouseButton) {
        mouseState.middlePressed = isPressed;
    }
    else if (key == EKeys::RightMouseButton) {
        mouseState.rightPressed = isPressed;
    }
    else if (key == EKeys::MouseScrollUp) {
        mouseState.wheelUp = isPressed;
    }
    else if (key == EKeys::MouseScrollDown) {
        mouseState.wheelDown = isPressed;
    }

    _event = mouseState;
    _event.x = uint16(localPosition.X * width);
    _event.y = uint16(localPosition.Y * height);

    return true;
}

bool Control::HandleKeyboardEvent(FKey key, bool isPressed, SKeyEvent& _event, bool isShiftHolding)
{
    uint32 code = 0;

    if (isShiftHolding && FKeysMap::FKeyToShiftedASCII.count(key.ToString()) == 1) {
        code = FKeysMap::FKeyToShiftedASCII[key.ToString()];
    }
    else if (FKeysMap::FKeyToASCII.count(key.ToString()) == 1) {
        code = FKeysMap::FKeyToASCII[key.ToString()];
    }

    if (code) {
        _event = {
            code,
            isPressed
        };
        return true;
    }
    return false;
}

void Control::SetSettings(
    APlayerController* _playerController,
    UStaticMeshComponent* _mesh,
    UWidgetComponent* _keyboardWidgetComponent,
    UWidgetInteractionComponent* _widgetInteractionComponent
)
{
    playerController = _playerController;
    mesh = _mesh;
    keyboardWidgetComponent = _keyboardWidgetComponent;
    if (keyboardWidgetComponent) {
        keyboardWidgetComponent->SetVisibility(false);
    }
    widgetInteractionComponent = _widgetInteractionComponent;
}