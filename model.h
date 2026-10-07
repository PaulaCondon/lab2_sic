#pragma once
#include <Arduino.h>
namespace Eloquent {
    namespace ML {
        namespace Port {
            class RandomForest {
                public:
                    /**
                    * Predict class for features vector
                    */
                    int predict(float *x) {
                        uint8_t votes[2] = { 0 };
                        // tree #1
                        if (x[1] <= 387.0) {
                            if (x[0] <= 87.19000244140625) {
                                votes[1] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        else {
                            if (x[2] <= 23.65000057220459) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #2
                        if (x[2] <= 23.65000057220459) {
                            votes[0] += 1;
                        }

                        else {
                            votes[1] += 1;
                        }

                        // tree #3
                        if (x[0] <= 87.19000244140625) {
                            votes[1] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #4
                        if (x[2] <= 23.65000057220459) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[0] <= 78.66500091552734) {
                                votes[1] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        // tree #5
                        if (x[1] <= 387.0) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[2] <= 23.65000057220459) {
                                votes[0] += 1;
                            }

                            else {
                                votes[1] += 1;
                            }
                        }

                        // tree #6
                        if (x[2] <= 23.65000057220459) {
                            votes[0] += 1;
                        }

                        else {
                            if (x[0] <= 78.66500091552734) {
                                votes[1] += 1;
                            }

                            else {
                                votes[0] += 1;
                            }
                        }

                        // tree #7
                        if (x[0] <= 78.66500091552734) {
                            votes[1] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #8
                        if (x[0] <= 87.19000244140625) {
                            votes[1] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #9
                        if (x[0] <= 87.19000244140625) {
                            votes[1] += 1;
                        }

                        else {
                            votes[0] += 1;
                        }

                        // tree #10
                        if (x[1] <= 381.5) {
                            votes[1] += 1;
                        }

                        else {
                            if (x[3] <= 64.5) {
                                votes[0] += 1;
                            }

                            else {
                                if (x[0] <= 75.77000045776367) {
                                    votes[1] += 1;
                                }

                                else {
                                    votes[0] += 1;
                                }
                            }
                        }

                        // return argmax of votes
                        uint8_t classIdx = 0;
                        float maxVotes = votes[0];

                        for (uint8_t i = 1; i < 2; i++) {
                            if (votes[i] > maxVotes) {
                                classIdx = i;
                                maxVotes = votes[i];
                            }
                        }

                        return classIdx;
                    }

                protected:
                };
            }
        }
    }