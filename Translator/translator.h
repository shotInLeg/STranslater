#ifndef TRANSLATOR_H
#define TRANSLATOR_H


#include <QString>
#include <QStringList>
#include <QVector>
#include <QFile>
#include <QDebug>

enum Speech{ NOUN = 1, VERB = 2, ADJ = 3, PROP = 4, UNION = 5, INF = 0 };
enum Sex{ MALE = 1, FEMALE = 2, MID = 3, MORE = 4 };
enum Lang{ RUS = 0, ENG = 1, LAT = 2 };
enum Case{ I = 0, R, D, V, T, P, Z1, Z2 };

using CaseList = QVector< QString >;

struct Word
{
    Speech speech;
    Sex sex;

    QVector< CaseList > cases;

    Word()
    {
        speech = Speech::NOUN;
        sex =  Sex::MALE;
        cases.resize(2);
    }

    Word(Speech s, Sex se, const CaseList& source, const CaseList& result)
    {
        speech = s;
        sex = se;
        cases.resize(2);
        cases[0] = source;
        cases[1] = result;
    }
};

class WordInfo
{
public:
    QVector<Word>* wordlist;
    int wordIndex;
    int translateIndex;
    int caseIndex;

    int pos;

    WordInfo()
    {
        wordIndex = -1;
        translateIndex = -1;
        caseIndex = -1;
        pos = -1;
        wordlist = NULL;
    }

    QString word() const
    {
        if( wordlist != NULL )
        {
            if( wordIndex != -1 && translateIndex != -1 && caseIndex != -1 )
            {
                return wordlist->at(wordIndex).cases[translateIndex].at(caseIndex);
            }
        }

        return "";
    }

    Sex sex() const
    {
        if( wordlist != NULL )
        {
            if( wordIndex != -1 && translateIndex != -1 && caseIndex != -1 )
            {
                return wordlist->at(wordIndex).sex;
            }
        }

        return Sex::MALE;
    }

    Speech speech() const
    {
        if( wordlist != NULL )
        {
            if( wordIndex != -1 && translateIndex != -1 && caseIndex != -1 )
            {
                return wordlist->at(wordIndex).speech;
            }
        }

        return Speech::INF;
    }

    Case case_() const
    {
        return Case(caseIndex);
    }

    QString translate() const
    {
        int translateIndex2 = (translateIndex==0)?1:0;

        if( wordlist != NULL )
        {
            if( wordIndex != -1 && translateIndex != -1 && caseIndex != -1 )
            {
                return wordlist->at(wordIndex).cases[translateIndex2].at(caseIndex);
            }
        }

        return "";
    }

    QVector<QString> getCaselist( int source = 0 ) const
    {
        return wordlist->at(wordIndex).cases[source];
    }
};

class Translator
{
public:
    Translator();
    Translator(const QString& text, Lang source);

    Translator& setText( const QString& text, Lang from );

    QString translate( Lang from, Lang to );

private:
    QString translate_(const QString& source_text, Lang source, Lang result );

    void makeWordList();

    Speech speechFromStr( const QString& str );
    Sex sexFromStr( const QString& str );
    QVector<QString> vectorFromStr( const QString& str );

    QVector<Word>& selectWordlist(int& case_source, int& case_result , Lang source, Lang result);
    QVector<WordInfo*> makeWordInfoList(const QVector<QString> text_vec, QVector<Word>* wordlist , int source_cases);
    QVector<WordInfo*> findPropWithNoun(const QVector<WordInfo*>& wordinfoList, int source_cases);
    QVector<WordInfo*> clearWordInfoList(const QVector<WordInfo*>& wordinfoList);
    QVector<WordInfo*> reducSeriaNoun(QVector<Word>& wordlist, const QVector<WordInfo*>& wordinfoList, int source_cases , int result_cases);
    QVector<WordInfo*> reducAdjAndNoun(QVector<Word>& wordlist, const QVector<WordInfo*>& wordinfoList, int &pos, int source_cases , int result_cases);
    QVector<WordInfo*> reducFreeNoun(const QVector<WordInfo*>& wordinfoList, int &pos);
    QVector<WordInfo*> reducFreeNounAfterVerb(const QVector<WordInfo*>& wordinfoList, int &pos, Case case_, int verb_pos );
    QVector<WordInfo*> fixVerbSex(QVector<Word>& wordlist, const QVector<WordInfo*>& wordinfoList , int source_cases);
    QVector<WordInfo*> findOtherSex(QVector<Word>& wordlist, const QVector<WordInfo*>& wordinfoList, int i, Sex sex, int source_cases  );

    void printDebug( const QVector<WordInfo*>& wordinfoList );

private:
    QString text;
    Lang lang;

    QVector<Word> eng_rus;
    QVector<Word> eng_lat;
};

#endif // TRANSLATOR_H
