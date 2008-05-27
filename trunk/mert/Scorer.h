#ifndef __SCORER_H__
#define __SCORER_H__

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include "Types.h"
#include "ScoreData.h"

using namespace std;

class ScoreStats;

/**
  * Superclass of all scorers and dummy implementation. In order to add a new
  * scorer it should be sufficient to override prepareStats(), setReferenceFiles()
  * and score() (or calculateScore()). 
**/
class Scorer {
  private:
	  string _name;
	
  public:
		
	Scorer(const string& name): _name(name), _scoreData(0),_preserveCase(false){ cerr << "I AM A SCORER" << std::endl;};
		virtual ~Scorer(){};
	
		/**
		  * set the reference files. This must be called before prepareStats.
		  **/
		virtual void setReferenceFiles(const vector<string>& referenceFiles) {
			//do nothing
		}

		/**
		 * Process the given guessed text, corresponding to the given reference sindex
         * and add the appropriate statistics to the entry.
		**/
		virtual void prepareStats(size_t sindex, const string& text, ScoreStats& entry)
		{
			cerr << "SCORER:prepareStats(size_t sindex, const string& text, ScoreStats& entry)" << text << std::endl;
		}

		virtual void prepareStats(const string& sindex, const string& text, ScoreStats& entry)
		{
			cerr << "SCORER:prepareStats(const string& sindex, const string& text, ScoreStats& entry)" << text << std::endl;
			this->prepareStats((size_t) atoi(sindex.c_str()), text, entry);
			//cerr << text << std::endl;
		}
				
        /**
          * Score using each of the candidate index, then go through the diffs
          * applying each in turn, and calculating a new score each time.
          **/
        virtual void score(const candidates_t& candidates, const diffs_t& diffs,
                statscores_t& scores) {
            //dummy impl
			if (!_scoreData) {
				throw runtime_error("score data not loaded");
			}
            scores.push_back(0);
            for (size_t i = 0; i < diffs.size(); ++i) {
                scores.push_back(0);
            }
        }


		/**
		  * Calculate the score of the sentences corresponding to the list of candidate
		  * indices. Each index indicates the 1-best choice from the n-best list.
		  **/
		float score(const candidates_t& candidates) {
            diffs_t diffs;
            statscores_t scores;
            score(candidates, diffs, scores);
            return scores[0];
		}

		const string& getName() const {return _name;}

        size_t getReferenceSize() {
            if (_scoreData) {
                return _scoreData->size();
            }
            return 0;
        }
		

		/**
		  * Set the score data, prior to scoring.
		  **/
		void setScoreData(ScoreData* data) {
			_scoreData = data;
		}

		protected:
      typedef map<string,int> encodings_t;
      typedef map<string,int>::iterator encodings_it;

			ScoreData* _scoreData;
	    encodings_t _encodings;

	    bool _preserveCase;

			
            /**
              * Tokenise line and encode.
              * 	Note: We assume that all tokens are separated by single spaces
              **/
            void encode(const string& line, vector<int>& encoded) {
                //cerr << line << endl;
                istringstream in (line);
                string token;
                while (in >> token) {
                    if (!_preserveCase) {
                        for (string::iterator i = token.begin(); i != token.end(); ++i) {
                            *i = tolower(*i);
                        }
                    }
                    encodings_it encoding = _encodings.find(token);
                    int encoded_token;
                    if (encoding == _encodings.end()) {
                        encoded_token = (int)_encodings.size();
                        _encodings[token] = encoded_token;
                        //cerr << encoded_token << "(n) ";
                    } else {
                        encoded_token = encoding->second;
                        //cerr << encoded_token << " ";
                    }
                    encoded.push_back(encoded_token);
                }
                //cerr << endl;
            }


};


/**
  * Abstract base class for scorers that work by adding statistics across all 
  * outout sentences, then apply some formula, e.g. bleu, per. **/
class StatisticsBasedScorer : public Scorer {

    public:
  StatisticsBasedScorer(const string& name): Scorer(name) {cerr << "I AM A StatisticsBasedScorer" << std::endl;}
	~StatisticsBasedScorer(){};
	virtual void score(const candidates_t& candidates, const diffs_t& diffs,
                statscores_t& scores);

protected:
        //calculate the actual score
        virtual statscore_t calculateScore(const vector<int>& totals) = 0;

};


#endif //__SCORER_H
