// Fill out your copyright notice in the Description page of Project Settings.

#include "Street.h"

Street::Street() :
	startNode(new Node()),
	endNode(new Node())
{
	startNode->owners.Add(this);
	startNode->radius = width * 0.5;

	endNode->owners.Add(this);
	endNode->radius = width * 0.5;
}

Street::Street(Node* sharedStartNode, Node* sharedEndNode) :
	startNode(sharedStartNode != nullptr ? sharedStartNode : (new Node())),
	endNode(sharedEndNode != nullptr ? sharedEndNode : (new Node()))
{
	startNode->owners.Add(this);
	startNode->radius = width * 0.5;

	endNode->owners.Add(this);
	endNode->radius = width * 0.5;
}

Street::~Street()
{
	if (startNode != nullptr)
	{
		startNode->owners.Remove(this);
		if (startNode->owners.Num() == 0) delete startNode;
	}

	if (endNode != nullptr)
	{
		endNode->owners.Remove(this);
		if (endNode->owners.Num() == 0) delete endNode;
	}

	for (Street* street : connectedStreets)
	{
		street->connectedStreets.Remove(this);
	}
}

void Street::UpdateStartNode(Node* newStartNode) 
{
	if (startNode != nullptr)
	{
		startNode->owners.Remove(this);
		if (startNode->owners.Num() == 0) delete startNode;
	}

	startNode = newStartNode;
	startNode->owners.Add(this);
}

void Street::UpdateEndNode(Node* newEndNode)
{
	if (endNode != nullptr)
	{
		endNode->owners.Remove(this);
		if (endNode->owners.Num() == 0) delete endNode;
	}

	endNode = newEndNode;
	endNode->owners.Add(this); //add this street to shared node
}

void Street::ConnectStreet(Street* streetToConnect) 
{
	if (!connectedStreets.Contains(streetToConnect))
		connectedStreets.Add(streetToConnect);

	for (Street* street : streetToConnect->GetConnectedStreets()) 
	{
		if(street != this && !connectedStreets.Contains(street))
			connectedStreets.Add(street);
	}
}

const TArray<Street*>& Street::GetConnectedStreets() 
{
	return connectedStreets;
}

void Street::ResetVerticesPositions(const Node* asossiatedNode)
{
	FRotator rot(0, 90, 0);
	FVector currentStreetDir = endNode->position - startNode->position;
	currentStreetDir.Normalize();
	FVector currentStreetTangent = rot.RotateVector(currentStreetDir);

	if (asossiatedNode == endNode) 
	{
		FVector currentStreetVertTopLeft = endNode->position + currentStreetTangent * -1 * width * 0.5 + FVector::UpVector * 10;
		FVector currentStreetVertTopRight = endNode->position + currentStreetTangent * width * 0.5 + FVector::UpVector * 10;

		vertices[2] = currentStreetVertTopLeft;
		vertices[3] = currentStreetVertTopRight;
	}
	else
	{
		FVector currentStreetVertDownLeft = startNode->position + currentStreetTangent * -1 * width * 0.5 + FVector::UpVector * 10;
		FVector currentStreetVertDownRight = startNode->position + currentStreetTangent * width * 0.5 + FVector::UpVector * 10;

		vertices[0] = currentStreetVertDownLeft;
		vertices[1] = currentStreetVertDownRight;
	}
}

TArray<FVector*> Street::GetVerticesForNode(const Node* node) 
{
	TArray<FVector*> selectedVertices;
	if (node == endNode) 
	{
		selectedVertices.Add(&vertices[2]);
		selectedVertices.Add(&vertices[3]);
	}
	else if (node == startNode) 
	{
		selectedVertices.Add(&vertices[0]);
		selectedVertices.Add(&vertices[1]);
	}
	return selectedVertices;
}