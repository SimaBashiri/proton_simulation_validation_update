#include "TFile.h"
#include "TProfile.h"
#include "TGraphErrors.h"
#include "TGraph2D.h"
#include "TMatrixD.h"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

using namespace std;

//----------------------------------------------------------------------------------------------------

TGraphErrors* MakeDiff(const TProfile *p1, const TProfile *p2)
{
    auto g = new TGraphErrors;

    for (int bi = 1; bi <= p1->GetNbinsX(); ++bi)
    {
        double xi = p1->GetBinCenter(bi);
        double xi_unc = p1->GetBinWidth(bi) / 2.0;

        double c1 = p1->GetBinContent(bi);
        double c1_unc = p1->GetBinError(bi);

        double c2 = p2->GetBinContent(bi);

        int idx = g->GetN();
        g->SetPoint(idx, xi, c1 - c2);
        g->SetPointError(idx, xi_unc, c1_unc);
    }

    return g;
}

//----------------------------------------------------------------------------------------------------

TGraphErrors* MakeCombination(const vector<TGraphErrors *> &graphs)
{
    auto g = new TGraphErrors(*graphs[0]);

    for (int i = 0; i < g->GetN(); ++i)
    {
        double sum2 = 0.0;

        for (const auto &graph : graphs)
        {
            double y = graph->GetY()[i];
            sum2 += y * y;
        }

        g->GetY()[i] = sqrt(sum2);
    }

    return g;
}

//----------------------------------------------------------------------------------------------------

TGraph2D* MakeCorrelation(const vector<TGraphErrors *> &graphs)
{
    auto g_ref = graphs[0];
    int dim = g_ref->GetN();

    TMatrixD V(dim, dim);
    for (const auto &graph : graphs)
    {
        for (int i = 0; i < dim; ++i)
        {
            for (int j = 0; j < dim; ++j)
            {
                double ci = graph->GetY()[i];
                double cj = graph->GetY()[j];

                V(i, j) += ci * cj;
            }
        }
    }

    TMatrixD C(dim, dim);
    for (int i = 0; i < dim; ++i)
    {
        for (int j = 0; j < dim; ++j)
        {
            double denom = V(i, i) * V(j, j);
            C(i, j) = (denom > 0.0) ? V(i, j) / sqrt(denom) : 0.0;
        }
    }

    auto g2_corr = new TGraph2D();
    g2_corr->SetTitle(";#xi;#xi");

    for (int i = 0; i < dim; ++i)
    {
        for (int j = 0; j < dim; ++j)
        {
            double xi = g_ref->GetX()[i];
            double xj = g_ref->GetX()[j];

            int idx = g2_corr->GetN();
            g2_corr->SetPoint(idx, xi, xj, C(i, j));
        }
    }

    return g2_corr;
}

//----------------------------------------------------------------------------------------------------

void PrintUsage()
{
    cout << "USAGE: collect_systematics <period>" << endl;
}

//----------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        PrintUsage();
        return 1;
    }

    string period = argv[1];
    string dir = "./";

    struct Scenario
    {
        string name;
        string simu_eff;
        string simu_none;
    };

    vector<Scenario> scenarios = {
        {"alig-x-sym", "proton_reco_misalignment/misalignment_x_sym_validation.root", "proton_reco_misalignment/misalignment_none_validation.root"},
        {"alig-x-asym", "proton_reco_misalignment/misalignment_x_asym_validation.root", "proton_reco_misalignment/misalignment_none_validation.root"},
        {"alig-y-sym", "proton_reco_misalignment/misalignment_y_sym_validation.root", "proton_reco_misalignment/misalignment_none_validation.root"},
        {"alig-y-asym", "proton_reco_misalignment/misalignment_y_asym_validation.root", "proton_reco_misalignment/misalignment_none_validation.root"},
        {"opt-Lx", "proton_reco_optics/optics_Lx_1_validation.root", "proton_reco_optics/optics_none_1_validation.root"},
        {"opt-Lpx", "proton_reco_optics/optics_Lpx_1_validation.root", "proton_reco_optics/optics_none_1_validation.root"},
        {"opt-Lpy", "proton_reco_optics/optics_Lpy_1_validation.root", "proton_reco_optics/optics_none_1_validation.root"},
        {"opt-xd", "proton_reco_optics/optics_xd_1_validation.root", "proton_reco_optics/optics_none_1_validation.root"},
    };

    vector<string> elements = {
        "single rp/2", "single rp/3", "single rp/23", "single rp/102", "single rp/103", "single rp/123",
        "multi rp/0", "multi rp/1"};

    vector<string> hist_names = {
        "p_de_t_vs_t_simu", "p_de_vtx_y_vs_xi_simu", "p_de_th_y_vs_xi_simu", 
        "p_de_th_x_vs_xi_simu", "p_de_t_vs_xi_simu", "p_de_xi_vs_xi_simu"};

    auto f_out = TFile::Open("collect_systematics.root", "recreate");

    for (const auto &el : elements)
    {
        string dn_el = el;
        replace(dn_el.begin(), dn_el.end(), '/', '-');

        cout << "* Processing: " << dn_el << endl;
        auto d_el = f_out->mkdir(dn_el.c_str());

        for (const auto &hist_name : hist_names)
        {
            vector<TGraphErrors *> graphs;

            for (const auto &sc : scenarios)
            {
                unique_ptr<TFile> f_in_eff(TFile::Open((dir + sc.simu_eff).c_str()));
                unique_ptr<TFile> f_in_none(TFile::Open((dir + sc.simu_none).c_str()));

                if (!f_in_eff || !f_in_none)
                {
                    cerr << "ERROR: Cannot open input files: " << sc.simu_eff << ", " << sc.simu_none << endl;
                    continue;
                }

                TProfile *p_eff = dynamic_cast<TProfile *>(f_in_eff->Get((el + "/" + hist_name).c_str()));
                TProfile *p_none = dynamic_cast<TProfile *>(f_in_none->Get((el + "/" + hist_name).c_str()));

                if (!p_eff || !p_none)
                {
                    cerr << "ERROR: Cannot load histograms for scenario " << sc.name << " and histogram " << hist_name << endl;
                    continue;
                }

                gDirectory = d_el;
                auto g_diff = MakeDiff(p_eff, p_none);
                g_diff->Write((sc.name + "_" + hist_name).c_str());
                graphs.push_back(g_diff);
            }

            if (!graphs.empty())
            {
                auto g_comb = MakeCombination(graphs);
                g_comb->Write(("combined_" + hist_name).c_str());

                auto g_corr = MakeCorrelation(graphs);
                g_corr->Write(("g2_correlation_" + hist_name).c_str());
            }
        }
    }

    delete f_out;
    return 0;
}
