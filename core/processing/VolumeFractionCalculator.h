#ifndef VOLUMEFRACTIONCALCULATOR_H
#define VOLUMEFRACTIONCALCULATOR_H

#include <vector>
#include <string>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>

class vtkCell;
class vtkDataArray;

/**
 * @brief 応力範囲ごとの体積分率を計算するクラス
 *
 * 解析結果の応力範囲を指定数の区間に等分割し、
 * 各区間に属するセルの体積寄与率（体積分率）を計算する。
 */
class VolumeFractionCalculator {
public:
    VolumeFractionCalculator();
    ~VolumeFractionCalculator() = default;

    /**
     * @brief 体積分率を計算する
     * @param vtuData 解析結果のUnstructuredGrid
     * @param stressLabel 応力データのラベル名
     * @param stressMin 応力範囲の最小値
     * @param stressMax 応力範囲の最大値
     * @param numDivisions 分割数（デフォルト: 20）
     * @return 計算成功ならtrue
     */
    bool compute(vtkUnstructuredGrid* vtuData,
                 const std::string& stressLabel,
                 double stressMin,
                 double stressMax,
                 int numDivisions = 20);

    // Getter
    const std::vector<double>& getVolumeFractions() const { return m_volumeFractions; }
    bool hasResult() const { return m_computed; }
    double getTotalVolume() const { return m_totalVolume; }
    int getNumDivisions() const { return static_cast<int>(m_volumeFractions.size()); }

    // リセット
    void clear();

private:
    // 入力データの検証
    bool validateInput(vtkUnstructuredGrid* vtuData,
                       const std::string& stressLabel,
                       double stressMin,
                       double stressMax);

    // セルの平均応力を計算
    double computeCellAverageStress(vtkCell* cell, vtkDataArray* stressArray);

    // セルの体積を計算（セルタイプに応じた処理）
    double computeCellVolume(vtkCell* cell);

    // 四面体の体積計算
    double computeTetraVolume(vtkCell* cell);

    // 六面体の体積計算
    double computeHexahedronVolume(vtkCell* cell);

    // 該当する区間インデックスを決定
    int determineBinIndex(double stress, double stressMin, double stressStep, int numDivisions);

private:
    std::vector<double> m_volumeFractions;  // 各区間の体積分率
    double m_totalVolume = 0.0;             // 全体体積
    bool m_computed = false;                // 計算済みフラグ
};

#endif // VOLUMEFRACTIONCALCULATOR_H
