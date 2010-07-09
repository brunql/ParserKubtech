 /*
  * Author: brunql
  * Creation date: 28.06.10
  *
  * Parse for kubtech.opennet.ru
  */


#include <QtCore>
#include <QtGui>
#include <iostream>

#define GROUP_ID_START  1
#define GROUP_ID_END    238
#define GROUP_COUNT     238

using namespace std;

QApplication *a;

QString findAndReplaceImages(QString line)
{
    line = line.replace(QRegExp(
            QString(a->trUtf8("\\<img align\\=\"absmiddle\" border\\=\"0\" src\\=\"\\/OpenUni\\/Questions\\/Questions-\\d+\\.nsf\\/QuestionsByID\\/00\\d+\\/\\$FILE\\/([^<>\\.\"]+\\.([^<>\\.\"]+))\"\\>")),
            Qt::CaseSensitive),
                        "<<<\\2=\\1>>>"
                        );
    return line;
}

QString clearString(QString line)
{
    line = line.replace("&gt;", ">", Qt::CaseSensitive);
    line = line.replace("&lt;", "<", Qt::CaseSensitive);
    line = line.replace("&nbsp;", " ", Qt::CaseSensitive);
    line = line.remove(QRegExp("\\<[^<>=]+\\>", Qt::CaseSensitive));
    return line;
}


void convertAll()
{
    QString newLine = "\r\n";
    QString fileOutPath = QDir::currentPath() + "/out/";
    QString fileInPath = QDir::currentPath() + "/in/";
    QString fileName = "";

    for(int groupID = GROUP_ID_START; groupID <= GROUP_ID_END; groupID++){
        cout << groupID << ". ";
        QString text = "";
        QFile readMeFile(fileInPath + QString::number(groupID));
        if(!readMeFile.open(QIODevice::ReadOnly)){
            qDebug() << "Error: Open file for read!";
        }

        int questionType = 1;

        QTextStream stream(&readMeFile);
        while(!stream.atEnd()){
            QString line = stream.readLine(0xffff);
            line = line.trimmed();

            if(line.contains(a->trUtf8(">&nbsp;<b>Вопрос:</b>"), Qt::CaseSensitive) &&
               questionType >= 1 && questionType <= 5)
            {
                text += "--/New Question/--" + newLine;
                text += "<$:QuestionType: " + QString::number(questionType) + newLine;

                text += "<$:QuestionBody: ";                
                // Start QuestionBody
                line = stream.readLine(0xffff);
                line = line.trimmed();
                line = findAndReplaceImages(line);
                while(line != "</div>"){
                    text += clearString(line)  + newLine;
                    line = stream.readLine(0xffff);
                    line = line.trimmed();
                    line = findAndReplaceImages(line);
                }
                // End QuestionBody

                if(questionType == 1 || questionType == 2){
                    text += "<$:Variants:" + newLine;
                    // Start Variants
                    line = stream.readLine(0xffff);
                    line = line.trimmed();
                    line = findAndReplaceImages(line);

                    QString rightAnswers = "";
                    while((!line.contains(a->trUtf8("</table>"))) && (!stream.atEnd())){
                        if(!line.startsWith('<') && line.length() != 0){

                            text += clearString(line) + newLine; // Find answer

                            // Check: right answer?
                            line = stream.readLine(0xffff);
                            line = line.trimmed();
                            if(line.contains(a->trUtf8("(верный)"))){
                                rightAnswers += " 1";
                            }else{
                                rightAnswers += " 0";
                            }
                        }

                        line = stream.readLine(0xffff);
                        line = line.trimmed();
                        line = findAndReplaceImages(line);
                    }
                    // End Variants

                    text += "<$:RightAnswers:" + rightAnswers + newLine;
                }else if(questionType == 5){
                    text += "<$:Variants:" + newLine;
                    // Start Variants
                    line = stream.readLine(0xffff);
                    line = line.trimmed();
                    line = findAndReplaceImages(line);

                    QString rightAnswers = "";
                    while((!line.contains(a->trUtf8("</tbody></table>"))) && (!stream.atEnd())){
                        if(line.startsWith("<tr id=")){
                            rightAnswers += " " + line[30]; // 30 - index of number in sequence
                        }else if(line.startsWith("<td>")){
                            text += clearString(line) + newLine; // Find answer, clearString - also clear all tags
                        }

                        line = stream.readLine(0xffff);
                        line = line.trimmed();
                        line = findAndReplaceImages(line);
                    }
                    // End Variants

                    text += "<$:RightAnswers:" + rightAnswers + newLine;
                }else if(questionType == 3){
                    text += "<$:EditableVariant: " + newLine;

                    // Start fing RightAnswer
                    while(!line.startsWith(a->trUtf8("<tr><td>Эталон правильного ответа:</td></tr>")) && (!stream.atEnd())){
                        line = stream.readLine(0xffff);
                        line = line.trimmed();
                    }
                    line = stream.readLine(0xffff);
                    line = line.trimmed();
                    line = findAndReplaceImages(line);
                    line.remove("<tr><td><font color=\"red\">", Qt::CaseSensitive);
                    line.remove("</font></td></tr>", Qt::CaseSensitive);

                    text += "<$:RightAnswer: " + line + newLine;
                }else if(questionType == 4){
                    /* Do nothing */
                }


                text += "<$:Difficulty: 3 " + newLine;
                text += "<$:AnswerTime: 600 " + newLine;
                text += "--/end/--" + newLine + newLine;


            }else if(line.contains(a->trUtf8("(Выбор из списка единственного правильного ответа)"))){
                questionType = 1;
                cout << " 1";
            }else if(line.contains(a->trUtf8("(Выбор из списка нескольких правильных ответов)"))){
                questionType = 2;
                cout << " 2";
            }else if(line.contains(a->trUtf8("(Ввод некоторых данных, которые впоследствии будут сравнены с заложенным эталоном)"))){
                questionType = 3;
                cout << " 3";
            }else if(line.contains(a->trUtf8("(Ответ на естественном языке)"))){
                questionType = 4;
                cout << " 4";
            }else if(line.contains(a->trUtf8("(Последовательность)"))){
                questionType = 5;
                cout << " 5";
            }else if(line.contains(a->trUtf8("(Выбор из списка единственного правильного ответа с полем \"Другой\")"))){
                questionType = 6;
                cout << " 6";
            }else if(line.contains(a->trUtf8("(Выбор по одному элементу в каждой колонке матрицы)"))){
                questionType = 7;
                cout << " 7";
            }else if(line.contains(
                    a->trUtf8("Список вопросов группы:</font> \"&nbsp;"),
                    Qt::CaseSensitive)){
                line = line.remove(0, line.indexOf("&nbsp;") + 6);
                int last = line.indexOf("&nbsp;\" </b></center><br>",0, Qt::CaseSensitive);
                line = line.remove(last, line.length() - last);
                text += line + newLine;

                // Convert file name:
                fileName = line.left(100).remove('"').replace(a->trUtf8("№"), "N", Qt::CaseSensitive).trimmed().remove(".txt").remove(':');
            }
        }

        QString fileOutName = fileOutPath /*+ QString::number(groupID +1) + ". "*/ + fileName + ".txt";
        QFile fileOut(fileOutName);
        if(fileOut.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            QTextStream stream(&fileOut);
            stream.setCodec("Windows-1251");
            stream << (text);
            stream.flush();
            fileOut.close();
        }else{
            qDebug() <<  "Error:" << "Error open file: " + fileOutName;
        }
        cout << endl;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    a = &app;


    qDebug() << "Parse for kubtech.opennet.ru start.";
    qDebug() << "Author: brunql \tDate: 28.06.2010";
    convertAll();
    qDebug() << "Parse for kubtech.opennet.ru end.";

    return 0;
}
