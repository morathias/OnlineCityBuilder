// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

enum StreetType;

class ONLINECITYBUILDER_API Street
{
public:
	struct Node
	{
	public:
		FVector position;
		float radius = 500;
		StreetType type = (StreetType)0;
		TArray<Street*> owners;
		TArray<int> holeIndices;
	};
	Node* startNode;
	Node* endNode;

	Street();
	Street(Node* sharedStartNode, Node* sharedEndNode);

	~Street();

	void UpdateStartNode(Node* newStartNode);
	void UpdateEndNode(Node* newEndNode);

	void ConnectStreet(Street* streetToConnect);
	const TArray<Street*>& GetConnectedStreets();

	void ResetVerticesPositions(const Node* asossiatedNode);

	TArray<FVector*> GetVerticesForNode(const Node* node);
	TArray<int> GetIndecesForNode(const Node* node);


	float width = 1000;

	TArray<FVector> vertices;
	TArray<int32> triangles;
	TArray<FVector2D> uvs;

private:
	TArray<Street*> connectedStreets;
};
