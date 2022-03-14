#include "cad.h"

#include <stdio.h>
#include "raymath.h"
#include "rayext.h"

Vector2 World2Part(Part part, Vector2 v) {
    return Vector2Rotate(Vector2Subtract(v, part.Position), -part.Angle * DEG2RAD);
}

Vector2 Part2World(Part part, Vector2 v) {
    return Vector2Add(Vector2Rotate(v, part.Angle * DEG2RAD), part.Position);
}

float WorldAngle2PartAngle(Part part, float angle) {
    return angle - part.Angle;
}

float PartAngle2WorldAngle(Part part, float angle) {
    return part.Angle + angle;
}

float attachmentCorrectionAngle(Vector2 attachPos, Vector2 COM) {
    return PI + atan2f(
        attachPos.x - COM.x,
        attachPos.y - COM.y
    );
}

Vector2 World2Attachment(Vector2 attachPos, Vector2 COM, Vector2 v) {
    float angle = attachmentCorrectionAngle(attachPos, COM);
    return Vector2Rotate(Vector2Subtract(v, attachPos), angle);
}

float WorldAngle2AttachmentAngle(Vector2 attachPos, Vector2 COM, float angle) {
    return angle + attachmentCorrectionAngle(attachPos, COM) * RAD2DEG;
}

float BoxMass(Box box) {
    if (box.HardcodedMass) {
        return box.Quantity * box.HardcodedMass;
    }
    
    float volume = box.Width * box.Height * box.Depth;
    if (box.WallThickness) {
        volume -= box.Width * (box.Height - 2*box.WallThickness) * (box.Depth - 2*box.WallThickness);
    }
    float density = 0;
    switch (box.Material) {
    case Aluminum: {
        density = 0.098f;
    } break;
    case Polycarb: {
        density = 0.0491f;
    } break;
    case Steel: {
        density = 0.2839f;
    } break;
    }

    return box.Quantity * volume * density;
}

void UpdateReferences(Part *part) {
    for (int b = 0; b < part->NumBoxes; b++) {
        Box *box = &part->Boxes[b];
        box->Part = part;
    }
}

float TotalMass(Part *parts, int numParts) {
    float totalMass = 0;
    
    for (int i = 0; i < numParts; i++) {
        Part *part = &parts[i];

        for (int i = 0; i < part->NumBoxes; i++) {
            Box box = part->Boxes[i];
            totalMass += BoxMass(box);
        }
    }

    return totalMass;
}

Vector2 ComputeCOM(Part *parts, int numParts) {
    float totalMass = TotalMass(parts, numParts);

    Vector2 COM = {0};
    for (int i = 0; i < numParts; i++) {
        Part *part = &parts[i];

        for (int i = 0; i < part->NumBoxes; i++) {
            Box box = part->Boxes[i];
            float mass = BoxMass(box);
            float weight = mass / totalMass;

            Vector2 worldPos = Part2World(*part, box.Position);

            COM = Vector2Add(COM, Vector2Scale(worldPos, weight));
        }
    }

    return COM;
}

void UpdatePartCOM(Part *part) {
    part->CenterOfMass = ComputeCOM(part, 1);
}
