//
// Created by VincentZhang on 2022-06-21.
//

#include "ShapeSegmentFrameState.h"
#include "Serialize/SerializeUtility.h"

IMPLEMENT_REGISTER_CLASS(ShapeSegmentFrameState, 10010);

IMPLEMENT_OBJECT_SERIALIZE(ShapeSegmentFrameState);

INSTANTIATE_TEMPLATE_TRANSFER(ShapeSegmentFrameState);

void ShapeSegmentFrameState::RecordSegments(int currentFrameId, float segmentBuffer[], int size) {
    SegmentKeyFrame *pKeyFrame = InsertOrUpdateKeyFrame(currentFrameId, this->m_KeyFrames);
    pKeyFrame->positionArray.resize(size);
    pKeyFrame->handleOutArray.resize(size);
    pKeyFrame->handleInArray.resize(size);

    for (int i = 0; i < size; i++) {
        pKeyFrame->positionArray[i].x = segmentBuffer[6 * i];
        pKeyFrame->positionArray[i].y = segmentBuffer[6 * i + 1];

        pKeyFrame->handleInArray[i].x = segmentBuffer[6 * i + 2];
        pKeyFrame->handleInArray[i].y = segmentBuffer[6 * i + 3];

        pKeyFrame->handleOutArray[i].x = segmentBuffer[6 * i + 4];
        pKeyFrame->handleOutArray[i].y = segmentBuffer[6 * i + 5];
    }

    Apply(currentFrameId);
}

std::vector<Vector3f> Lerp(std::vector<Vector3f> &k1, std::vector<Vector3f> &k2, float ratio) {
    if (k1.size() != k2.size()) {
        printf("Error!!!! Size doesn't match:%d != %d\n", k1.size(), k2.size());
        return std::vector<Vector3f>();
    }

    std::vector<Vector3f> resultVector(k1.size());
    for (int i = 0; i < k1.size(); i++) {
        resultVector[i] = Lerp(k1[i], k2[i], ratio);
    }
    return resultVector;
}

template<class TransferFunction>
void ShapeSegmentFrameState::Transfer(TransferFunction &transfer) {
    Super::Transfer(transfer);
    TRANSFER(m_KeyFrames);
}

bool ShapeSegmentFrameState::Apply(int frameId) {
    std::pair<SegmentKeyFrame *, SegmentKeyFrame *> resultKeyFrames;
    if (FindKeyFramePair(frameId, this->m_KeyFrames, resultKeyFrames)) {
        this->isValidFrame = true;
        SegmentKeyFrame *k1 = resultKeyFrames.first;
        SegmentKeyFrame *k2 = resultKeyFrames.second;

        if (k2 == NULL || k1->frameId == k2->frameId) {
            this->m_currentPositionArray = k1->positionArray;
            this->m_currentHandleInArray = k1->handleInArray;
            this->m_currentHandleOutArray = k1->handleOutArray;
        } else {
            float ratio = float(frameId - k1->frameId) / float(k2->frameId - k1->frameId);

            this->m_currentPositionArray = Lerp(k1->positionArray, k2->positionArray, ratio);
            this->m_currentHandleInArray = Lerp(k1->handleInArray, k2->handleInArray, ratio);
            this->m_currentHandleOutArray = Lerp(k1->handleOutArray, k2->handleOutArray, ratio);
        }
        return true;
    }

    return false;
}