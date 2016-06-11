#include "translator.h"

Translator::Translator()
{
    text = "";
    lang = ENG;

    makeWordList();
}

Translator::Translator(const QString &text, Lang source)
{
    this->text = text;
    lang = source;

    makeWordList();
}

Translator &Translator::setText(const QString &text, Lang from)
{
    this->text = text;
    lang = from;

    return *this;
}

QString Translator::translate(Lang from, Lang to)
{
    QString res = "";
    if( (from == Lang::LAT && to == Lang::RUS) || (from == Lang::RUS && to == Lang::LAT) )
    {
        QString t = translate_(text, from, Lang::ENG);
        res = translate_(t, Lang::ENG, to);
    }
    else
    {
        res = translate_(text, from, to);
    }

    return res;
}

QString Translator::translate_(const QString &source_text, Lang source, Lang result)
{
    int source_cases  = 0;
    int result_cases = 1;
    QVector<Word> wordlist = selectWordlist(source_cases, result_cases, source, result);
    QVector<QString> text_vec = vectorFromStr(source_text);
    QVector< WordInfo* > wordInfoList = makeWordInfoList(text_vec, &wordlist, source_cases);

    qDebug() << "Start";
    printDebug( wordInfoList );

    //Объединим передлоги для Р и П падежей со стловами
    wordInfoList = findPropWithNoun(wordInfoList, source_cases);

    //Чистим список слов
    wordInfoList = clearWordInfoList(wordInfoList);

    //Выводим состояние в дебаг
    // printDebug( wordInfoList );

    //Склеиваем однородные члены для создания множественного числа
    wordInfoList = reducSeriaNoun( wordlist, wordInfoList, source_cases, result_cases );

    //Выводим состояние в дебаг
    printDebug( wordInfoList );



    qDebug() << "ADJ";
    int pos = 0;

    //Прилагательные и связанные с ними существительные
    wordInfoList = reducAdjAndNoun(wordlist, wordInfoList, pos, source_cases, result_cases);

    //Выводим состояние в дебаг
    printDebug( wordInfoList );

    //Задаем позицию для свободных подлежащих
    wordInfoList = reducFreeNoun( wordInfoList, pos );

    //узнаем падеж и позицию глагола
    Case case_ = Case::I; int verb_pos = -1;
    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        if( wordInfoList.at(i)->speech() == Speech::VERB )
        {
            if( wordInfoList.at(i)->pos < 999 )
            {
                case_ = wordInfoList.at(i)->case_();
                wordInfoList.at(i)->pos = pos + 1000;
                verb_pos = pos;
                pos++;

                break;
            }
        }
    }

    /*for( int i = 0; i < wordInfoList.size(); i++ )
    {
        for( int j = i+1; j < wordInfoList.size(); j++ )
        {
            if( wordInfoList.at(j)->speech() == Speech::VERB )
                break;

            if( wordInfoList.at(i)->speech() == Speech::ADJ )
            {
                if( wordInfoList.at(i)->pos < 999 && wordInfoList.at(j)->pos < 999
                        && wordInfoList.at(j)->speech() == Speech::NOUN
                        && wordInfoList.at(i)->pos > verb_pos && verb_pos >= 0 && wordInfoList.at(i)->case_() != Case::P )
                {

                    qDebug() << "FIIIIIND >?.>>>?>?>??";
                    wordInfoList.at(i)->caseIndex = case_;
                    wordInfoList.at(j)->caseIndex = case_;
                    wordInfoList.at(i)->pos = pos + 1000;
                    pos++;
                    wordInfoList.at(j)->pos = pos + 1000;
                    pos++;

                    if( wordInfoList.at(i)->sex() != wordInfoList.at(j)->sex() )
                    {
                        wordInfoList = findOtherSex( wordlist, wordInfoList, i, wordInfoList.at(j)->sex(), source_cases );
                    }

                    break;
                }
                else if( verb_pos < 0 || wordInfoList.at(i)->case_() == Case::P)
                {
                    qDebug() << "INVERT " << wordInfoList.at(i)->word() << wordInfoList.at(i)->pos;

                    wordInfoList.at(i)->pos = pos + 1000;
                    pos++;

                    break;
                }
            }
        }

        int a = 5;
    }*/

    //выставление позиции для свободных дополнений
    wordInfoList = reducFreeNounAfterVerb(wordInfoList, pos, case_, verb_pos);

    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        wordInfoList.at(i)->pos = wordInfoList.at(i)->pos - 1000;
    }

    //изменение пола для глаголов
    wordInfoList = fixVerbSex(wordlist, wordInfoList, source_cases );


    int max_pos = 0;
    for(int i = 0; i < wordInfoList.size(); i++)
    {
        if( wordInfoList.at(i)->pos > max_pos )
            max_pos = wordInfoList.at(i)->pos;
    }

    QString trn = "";
    pos = 0;
    while(pos <= max_pos)
    {
        for(int j = 0; j < wordInfoList.size(); j++)
        {
            if( wordInfoList.at(j)->pos == pos )
            {
                trn += wordInfoList.at(j)->translate() + " ";
                break;
            }
        }
        pos++;
    }

    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        if(  wordInfoList.at(i)->pos < 0 )
        {
            trn += wordInfoList.at(i)->translate() + " ";
        }
    }

    return trn;
}

void Translator::makeWordList()
{
    //qDebug() << "Making...";
    QFile eng_rus_wordfile(":/lang/eng_rus.lang");
    QFile eng_lat_wordfile(":/lang/eng_lat.lang");

    if( eng_rus_wordfile.open(QIODevice::ReadOnly) )
    {
        //qDebug() << "File Opened";
        while( !eng_rus_wordfile.atEnd() )
        {
            //qDebug() << "Reading line";
            QString str = eng_rus_wordfile.readLine();
            QStringList lst = str.split("||");

            //qDebug() << lst.size();
            if(lst.size() < 19)
                continue;

            Word wrd;
            wrd.speech = speechFromStr( lst.at(0) );
            wrd.sex = sexFromStr( lst.at(1) );

            CaseList tmp;
            for(int i = 2; i < 10; i++)
                tmp.push_back( lst.at(i) );

            wrd.cases[0] = tmp;

            //            for( int i = 0; i < wrd.cases[0].size(); i++ )
            //                qDebug() << wrd.cases[0].at(i);

            tmp.clear();
            for(int i = 10; i < 18; i++)
                tmp.push_back( lst.at(i) );

            wrd.cases[1] = tmp;

            //            for( int i = 0; i < wrd.cases[1].size(); i++ )
            //                qDebug() << wrd.cases[1].at(i);

            eng_rus.push_back(wrd);
        }
    }

    if( eng_lat_wordfile.open(QIODevice::ReadOnly) )
    {
        while( !eng_lat_wordfile.atEnd() )
        {
            QString str = eng_lat_wordfile.readLine();
            QStringList lst = str.split("||");

            if(lst.size() < 19)
                continue;

            Word wrd;
            wrd.speech = speechFromStr( lst.at(0) );
            wrd.sex = sexFromStr( lst.at(1) );

            CaseList tmp;
            for(int i = 2; i < 10; i++)
                tmp.push_back( lst.at(i) );

            wrd.cases[0] = tmp;

            tmp.clear();
            for(int i = 10; i < 18; i++)
                tmp.push_back( lst.at(i) );

            wrd.cases[1] = tmp;

            eng_lat.push_back(wrd);

//            for( int i = 0; i < wrd.cases[0].size(); i++ )
//                qDebug() << wrd.cases[0].at(i);

//            for( int i = 0; i < wrd.cases[1].size(); i++ )
//                qDebug() << wrd.cases[1].at(i);

        }
    }

    eng_rus_wordfile.close();
    eng_lat_wordfile.close();

    qDebug() << "ENG_RUS::";
    for(int i = 0; i < eng_rus.size(); i++)
        for( int j = 0; j < eng_rus.at(i).cases[0].size(); j++ )
        {
            qDebug() << eng_rus.at(i).speech << eng_rus.at(i).sex << eng_rus.at(i).cases[0].at(j) << eng_rus.at(i).cases[1].at(j);
        }

    qDebug() << "ENG_LAT::";
    for(int i = 0; i < eng_rus.size(); i++)
        for( int j = 0; j < eng_lat.at(i).cases[0].size(); j++ )
        {
            qDebug() << eng_lat.at(i).speech << eng_lat.at(i).sex << eng_lat.at(i).cases[0].at(j) << eng_lat.at(i).cases[1].at(j);
        }
}

Speech Translator::speechFromStr(const QString &str)
{
    if( str == "noun" )
        return Speech::NOUN;
    else if( str == "verb" )
        return Speech::VERB;
    else if( str == "adj" )
        return Speech::ADJ;
    else if( str == "prop" )
        return Speech::PROP;
    else if( str == "union" )
        return Speech::UNION;

    return Speech::INF;
}

Sex Translator::sexFromStr(const QString &str)
{
    if( str == "male" )
        return Sex::MALE;
    else if( str == "female" )
        return Sex::FEMALE;
    else if( str == "more" )
        return Sex::MORE;
    else if( str == "mid" )
        return Sex::MID;

    return Sex::MALE;
}

QVector<QString> Translator::vectorFromStr(const QString &str)
{
    QVector<QString> vect;
    QStringList list = str.split(" ");

    for(int i = 0; i < list.size(); i++)
    {
        vect.push_back( list.at(i) );
    }

    return vect;
}

QVector<Word> &Translator::selectWordlist(int &case_source, int &case_result, Lang source, Lang result)
{
    case_source  = 0;
    case_result = 1;

    if( source == Lang::RUS && result == Lang::ENG )
    {
        case_source = 1;
        case_result = 0;
        return eng_rus;
    }
    else if( source == Lang::ENG && result == Lang::RUS )
    {
        case_source = 0;
        case_result = 1;
        return eng_rus;
    }
    else if( source == Lang::ENG && result == Lang::LAT )
    {
        case_source = 0;
        case_result = 1;
        return eng_lat;
    }
    else if( source == Lang::LAT && result == Lang::ENG )
    {
        case_source = 1;
        case_result = 0;
        return eng_lat;
    }
}

QVector<WordInfo *> Translator::makeWordInfoList(const QVector<QString> text_vec, QVector<Word>* wordlist, int source_cases)
{
    bool rec = false;
    QVector< WordInfo* > wordInfoList;
    for(int i = 0; i < text_vec.size(); i++)
    {
        for(int j = 0; j < wordlist->size(); j++)
        {
            rec = false;

            for(int k = 0; k < wordlist->at(j).cases[source_cases].size(); k++)
            {
                QString current_word = wordlist->at(j).cases[source_cases].at(k);
                qDebug() << text_vec.at(i) << current_word << j << k;

                if( current_word == text_vec.at(i) )
                {
                    //qDebug() << ">>>>>>" << current_word << text_vec.at(i);

                    WordInfo * wordInfo = new WordInfo;
                    wordInfo->pos = i;
                    wordInfo->wordlist = wordlist;
                    wordInfo->wordIndex = j;
                    wordInfo->translateIndex = source_cases;
                    wordInfo->caseIndex = k;

                    wordInfoList.push_back( wordInfo );

                    rec = true;
                    break;
                }
            }

            if( rec == true )
            {
                break;
            }
        }
    }

    return wordInfoList;
}

QVector<WordInfo *> Translator::findPropWithNoun(const QVector<WordInfo*>& wordinfoList, int source_cases)
{
    QVector<WordInfo*> wordInfoList = wordinfoList;

    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        if( wordInfoList.at(i)->speech() == Speech::PROP )
        {
            qDebug() << "Find prop" << wordInfoList.at(i)->word();

            for( int j = i+1; j < wordInfoList.size(); j++ )
            {
                QString cur_word_r = wordInfoList.at(i)->word() + " " + wordInfoList.at(j)->getCaselist(source_cases).at(6);
                QString cur_word_p = wordInfoList.at(i)->word() + " " + wordInfoList.at(j)->getCaselist(source_cases).at(7);

                QString current_word_r = wordInfoList.at(j)->getCaselist(source_cases).at(1);
                QString current_word_p = wordInfoList.at(j)->getCaselist(source_cases).at(5);

                //qDebug() << cur_word_r << current_word_r;
                //qDebug() << cur_word_p << current_word_p;

                if( current_word_r == cur_word_r )
                {
                    //qDebug() << "R";
                    wordInfoList.at(j)->caseIndex = 1;
                    wordInfoList.at(i)->pos = -1;
                    break;
                }
                if( current_word_p == cur_word_p )
                {
                    wordInfoList.at(j)->caseIndex = 5;
                    wordInfoList.at(i)->pos = -1;

                    //qDebug() << "P" << wordInfoList.at(j)->caseIndex;
                    break;
                }
            }
        }
    }

    return wordInfoList;
}

QVector<WordInfo *> Translator::clearWordInfoList(const QVector<WordInfo *> &wordinfoList)
{
    QVector< WordInfo* > tmp;
    for( int i = 0; i < wordinfoList.size(); i++ )
    {
        if( wordinfoList.at(i)->pos != -1 )
            tmp.push_back( wordinfoList.at(i) );
    }

    return tmp;
}

QVector<WordInfo *> Translator::reducSeriaNoun(QVector<Word> &wordlist, const QVector<WordInfo *> &wordinfoList, int source_cases, int result_cases)
{
    QVector<WordInfo*> wordInfoList = wordinfoList;

    int count_mn = 0; int first_mn = -1;
    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        //qDebug() << wordInfoList.at(i)->word();
        if( ((wordInfoList.at(i)->speech() == Speech::NOUN ) || (wordInfoList.at(i)->speech() == Speech::UNION)) && i != wordInfoList.size()-1 )
        {
            if( count_mn == 0 )
                first_mn = i;

            count_mn++;

            //qDebug() << "      true";
        }
        else if( count_mn != 0 )
        {
            //qDebug() << i << count_mn;

            if( i == wordInfoList.size()-1 && ((wordInfoList.at(i)->speech() == Speech::NOUN ) || (wordInfoList.at(i)->speech() == Speech::UNION)) )
            {
                count_mn++;
            }

            if( count_mn > 1 )
            {
                CaseList cases_s;
                CaseList cases_r;


                for(int j = 0, i = first_mn; j < wordInfoList.at(i)->getCaselist(source_cases).size(); j++ )
                {
                    QString more_n_source = "";
                    QString more_n_result = "";
                    for( ; i < first_mn+count_mn; i++ )
                    {

                        more_n_source += wordInfoList.at(i)->getCaselist(source_cases).at(j) + " ";
                        more_n_result += wordInfoList.at(i)->getCaselist(result_cases).at(j) + " ";
                    }
                    cases_s.push_back( more_n_source );
                    cases_r.push_back( more_n_result );
                    i = first_mn;
                }

                if( source_cases == 0 )
                    wordlist.push_back( Word(Speech::NOUN, Sex::MORE, cases_s, cases_r) );
                else
                    wordlist.push_back( Word(Speech::NOUN, Sex::MORE, cases_r, cases_s) );

                WordInfo * wordInfo = new WordInfo;
                wordInfo->pos = wordInfoList.at(first_mn)->pos;
                wordInfo->wordlist = &wordlist;
                wordInfo->wordIndex = wordlist.size()-1;
                wordInfo->translateIndex = source_cases;
                wordInfo->caseIndex = 0;

                wordInfoList.insert(first_mn, wordInfo);


                for( int i = first_mn+1; i < first_mn+count_mn+1; i++ )
                {
                    wordInfoList.remove(first_mn+1);
                }
            }

            //qDebug() << "      recodr";

            i = first_mn;
            count_mn = 0;
        }
    }

    return wordInfoList;
}

QVector<WordInfo *> Translator::reducAdjAndNoun(QVector<Word> &wordlist, const QVector<WordInfo *> &wordinfoList, int& pos, int source_cases, int result_cases)
{
    QVector<WordInfo*> wordInfoList = wordinfoList;

    bool can_mark = true;
    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        if( wordInfoList.at(i)->speech() == Speech::ADJ )
        {
            bool finded = false;

            for( int j = i+1; j < wordInfoList.size(); j++  )
            {
                if( wordInfoList.at(j)->speech() == Speech::VERB )
                {
                    can_mark = false;
                    break;
                }

                if( wordInfoList.at(i)->pos < 999 && wordInfoList.at(j)->pos < 999 && wordInfoList.at(j)->speech() == Speech::NOUN )
                {
                    finded = true;

                    int adj_noun_pos = ( wordInfoList.at(j)->pos < wordInfoList.at(i)->pos )?wordInfoList.at(j)->pos:wordInfoList.at(j)->pos;
                    Case noun_case = wordInfoList.at(j)->case_();
                    Sex noun_sex = wordInfoList.at(j)->sex();

                    if( wordInfoList.at(i)->sex() != noun_sex)
                    {
                        //Заменяем пол прилогательгного на пол существительного
                        wordInfoList = findOtherSex( wordlist, wordInfoList, i, noun_sex, source_cases );
                    }


                    int first_mn = i;

                    CaseList cases_s;
                    CaseList cases_r;

                    for(int k = 0, c = first_mn; k < wordInfoList.at(c)->getCaselist(source_cases).size(); k++ )
                    {
                        QString more_n_source = "";
                        QString more_n_result = "";
                        for( ; c < first_mn+2; c++ )
                        {

                            more_n_source += wordInfoList.at(c)->getCaselist(source_cases).at(k) + " ";
                            more_n_result += wordInfoList.at(c)->getCaselist(result_cases).at(k) + " ";
                        }
                        cases_s.push_back( more_n_source );
                        cases_r.push_back( more_n_result );

                        c = first_mn;
                    }

                    if( source_cases == 0 )
                        wordlist.push_back( Word(Speech::NOUN, Sex::MORE, cases_s, cases_r) );
                    else
                        wordlist.push_back( Word(Speech::NOUN, Sex::MORE, cases_r, cases_s) );

                    WordInfo * wordInfo = new WordInfo;
                    wordInfo->pos = wordInfoList.at(first_mn)->pos;
                    wordInfo->wordlist = &wordlist;
                    wordInfo->wordIndex = wordlist.size()-1;
                    wordInfo->translateIndex = source_cases;
                    wordInfo->caseIndex = 0;

                    wordInfoList.insert(first_mn, wordInfo);


                    for( int i = first_mn+1; i < first_mn+2+1; i++ )
                    {
                        wordInfoList.remove(first_mn+1);
                    }


                    wordInfoList.at(i)->pos = adj_noun_pos;
                    wordInfoList.at(i)->caseIndex = noun_case;

                    if( can_mark )
                    {
                        wordInfoList.at(i)->pos = pos + 1000;
                    }

                    pos++;

                    break;
                }
            }

            if( !finded )
            {
                wordInfoList.at(i)->pos = pos + 1000;
            }
        }
        else if( wordInfoList.at(i)->speech() == Speech::VERB )
        {
            can_mark = false;
            continue;
        }
    }

    return wordInfoList;
}

QVector<WordInfo *> Translator::reducFreeNoun(const QVector<WordInfo *> &wordinfoList, int &pos)
{
    QVector<WordInfo*> wordInfoList = wordinfoList;

    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        if( wordInfoList.at(i)->speech() == Speech::VERB )
            break;

        if( wordInfoList.at(i)->speech() == Speech::NOUN && wordInfoList.at(i)->caseIndex == Case::I )
        {
            if( wordInfoList.at(i)->pos < 999 )
            {
                //qDebug() << "INVERT " << wordInfoList.at(i)->word() << wordInfoList.at(i)->pos;

                wordInfoList.at(i)->pos = pos + 1000;
                pos++;

                break;
            }
        }
    }

    return wordInfoList;
}

QVector<WordInfo *> Translator::reducFreeNounAfterVerb(const QVector<WordInfo *> &wordinfoList, int &pos, Case case_, int verb_pos)
{
    QVector<WordInfo*> wordInfoList = wordinfoList;

    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        if( wordInfoList.at(i)->speech() == Speech::NOUN  )
        {
            //qDebug() << ">> " << wordInfoList.at(i)->word() << wordInfoList.at(i)->pos;

            if( wordInfoList.at(i)->pos < 999 && wordInfoList.at(i)->pos > verb_pos && verb_pos >= 0  && wordInfoList.at(i)->case_() != Case::P )
            {
                //qDebug() << "INVERT " << wordInfoList.at(i)->word() << wordInfoList.at(i)->pos;

                wordInfoList.at(i)->pos = pos + 1000;
                wordInfoList.at(i)->caseIndex = case_;
                pos++;

                break;
            }
            else if( verb_pos < 0 || wordInfoList.at(i)->case_() == Case::P)
            {
                //qDebug() << "INVERT " << wordInfoList.at(i)->word() << wordInfoList.at(i)->pos;

                wordInfoList.at(i)->pos = pos + 1000;
                pos++;

                break;
            }
        }
    }

    return wordInfoList;
}

QVector<WordInfo *> Translator::fixVerbSex(QVector<Word> &wordlist, const QVector<WordInfo *> &wordinfoList, int source_cases)
{
    QVector<WordInfo*> wordInfoList = wordinfoList;

    for( int i = 0; i < wordInfoList.size(); i++ )
    {
        if( wordInfoList.at(i)->speech() == Speech::NOUN && wordInfoList.at(i)->caseIndex == Case::I )
        {
            for( int j = i+1; j < wordInfoList.size(); j++ )
            {
                if( wordInfoList.at(j)->speech() == Speech::VERB && wordInfoList.at(j)->sex() !=  wordInfoList.at(i)->sex() )
                {
                    for( int k = 0; k < wordlist.size(); k++ )
                    {
                        if( wordInfoList.at(j)->getCaselist(source_cases).at(6) == wordlist.at(k).cases[source_cases].at(6) )
                        {
                            if( wordlist.at(k).sex == wordInfoList.at(i)->sex() )
                            {
                                wordInfoList.at(j)->wordIndex = k;
                            }
                        }
                    }
                }
            }
        }
    }

    return wordInfoList;
}

QVector<WordInfo *> Translator::findOtherSex(QVector<Word> &wordlist, const QVector<WordInfo *> &wordinfoList, int i, Sex sex, int source_cases)
{
    QVector<WordInfo*> wordInfoList = wordinfoList;

    for( int k = 0; k < wordlist.size(); k++ )
    {
        QString cur_adj = wordInfoList.at(i)->getCaselist(source_cases).at(6);
        QString list_adj = wordlist.at(k).cases[source_cases].at(6);
        Sex list_adj_sex = wordlist.at(k).sex;

        if( cur_adj == list_adj &&  list_adj_sex == sex )
        {
            wordInfoList.at(i)->wordIndex = k;
            break;
        }
    }

    return wordInfoList;
}

void Translator::printDebug(const QVector<WordInfo *> &wordinfoList)
{
    for(int i = 0; i < wordinfoList.size(); i++)
    {
        qDebug() << "i:" << i << "pos:" << wordinfoList.at(i)->pos << "sp:" << wordinfoList.at(i)->speech() <<  "sex:" << wordinfoList.at(i)->sex() << "case:" << wordinfoList.at(i)->case_() << wordinfoList.at(i)->word() << wordinfoList.at(i)->translate();
    }
}


//||mother||of mother||mother||mother||mother||about mother||mother||mother
//||мама  ||мамы     ||маме  ||маму  ||мамой ||о маме      ||мамы  ||маме
