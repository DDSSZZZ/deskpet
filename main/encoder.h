#ifndef ENCODER_H
#define ENCODER_H

void Encoder_Init();
void Encoder_Update();

bool Encoder_FeedRequest();
void Encoder_ClearFeedRequest();

#endif