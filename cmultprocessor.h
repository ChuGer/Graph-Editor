#ifndef CMULTPROCESSOR_H
#define CMULTPROCESSOR_H
#include <QList>
#include "graphscene.h"
#include <QProgressBar>

class CMultProcessor
{
public:
    CMultProcessor(QList<QList<int> > matrix, QStringList nodes, QProgressBar *);          // ����������� �������� ����������� �� ��������� ��������
    CMultProcessor( CMultProcessor &proc ); // ����������� �������� ����� ������������ �� �������������

    ~CMultProcessor();                      // ����������
    CMultProcessor* Clone();                // �������� ������������

    void GetAlts(QList<int>& );             // ��������� ����������� �������� ���������
    void AddLink( int ind );               // ���������� ��������� ������� � �������

    bool IsCycle();                         // �������� �� ����������� ����
    bool IsValid( bool first = true );      // �������� �������� ����� �� ������������
    QProgressBar *prB;

    QStringList ThreadMultiFind();
    void printMatrix(QList<QList<int> > m);
    void addToResult(CMultProcessor*);
protected:

    QStringList vertexes,result;
    QList<QList<int> > matr;                              // ������� ���������
    int count,begin,end;                               // ������ ������� ���������

    void __ClearEx( int from, int to );     // ���������� ������ ����� ����� ���������� � ������� �����
    void __CheckAll();                        // �������� ����������� �����
    int __GetEx( int from );                // ��������� ���������� �������� �� ��������������� ����
    int __GetNext( int from );              // ��������� ���������� �������� �������
    void  __GetList(QList<QString>&);              // ��������� ������ �����
};

#endif // CMULTPROCESSOR_H
