#include <QApplication>
#include <QComboBox>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

#undef emit
#include "Syllable.h"

using namespace vnpun;

class MainWindow : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle("Vnpun Playground");
        resize(720, 420);

        auto* central = new QWidget;
        setCentralWidget(central);

        auto* root = new QVBoxLayout(central);
        root->setContentsMargins(20, 20, 20, 20);
        root->setSpacing(16);

        // =========================================================
        // Title
        // =========================================================
        auto* title = new QLabel("Vietnamese Syllable Playground");
        title->setObjectName("title");

        auto* subtitle = new QLabel(
            "Parse UTF8 <-> normalized syllable"
        );
        subtitle->setObjectName("subtitle");

        root->addWidget(title);
        root->addWidget(subtitle);

        // =========================================================
        // Input section
        // =========================================================
        auto* inputCard = createCard();
        auto* inputLayout = new QVBoxLayout(inputCard);

        auto* inputLabel = new QLabel("Input UTF8");
        inputLabel->setObjectName("sectionLabel");

        inputEdit = new QLineEdit;
        inputEdit->setPlaceholderText("Ví dụ: chuyện, nguyễn, quốc...");

        auto* inputButtons = new QHBoxLayout;

        auto* parseBtn = new QPushButton("Parse");
        auto* clearBtn = new QPushButton("Clear");
        auto* swapBtn  = new QPushButton("Swap");

        inputButtons->addWidget(parseBtn);
        inputButtons->addWidget(clearBtn);
        inputButtons->addWidget(swapBtn);
        inputButtons->addStretch();

        inputLayout->addWidget(inputLabel);
        inputLayout->addWidget(inputEdit);
        inputLayout->addLayout(inputButtons);

        root->addWidget(inputCard);

        // =========================================================
        // Main content
        // =========================================================
        auto* contentLayout = new QHBoxLayout;
        contentLayout->setSpacing(16);

        // =========================================================
        // Left card - fields
        // =========================================================
        auto* fieldsCard = createCard();
        auto* fieldsLayout = new QVBoxLayout(fieldsCard);

        auto* parsedLabel = new QLabel("Normalized Components");
        parsedLabel->setObjectName("sectionLabel");

        auto* form = new QFormLayout;
        form->setVerticalSpacing(12);
        form->setHorizontalSpacing(12);

        phuAmDauEdit = new QLineEdit;
        amDemEdit    = new QLineEdit;
        amChinhEdit  = new QLineEdit;
        amCuoiEdit   = new QLineEdit;

        toneCombo = new QComboBox;
        toneCombo->addItems({
            "ngang",
            "sắc",
            "huyền",
            "hỏi",
            "ngã",
            "nặng"
        });

        form->addRow("Phụ âm đầu", phuAmDauEdit);
        form->addRow("Âm đệm", amDemEdit);
        form->addRow("Âm chính", amChinhEdit);
        form->addRow("Âm cuối", amCuoiEdit);
        form->addRow("Thanh", toneCombo);

        auto* generateBtn = new QPushButton("Generate UTF8");

        fieldsLayout->addWidget(parsedLabel);
        fieldsLayout->addLayout(form);
        fieldsLayout->addSpacing(8);
        fieldsLayout->addWidget(generateBtn);

        contentLayout->addWidget(fieldsCard, 1);

        // =========================================================
        // Right card - output
        // =========================================================
        auto* outputCard = createCard();
        auto* outputLayout = new QVBoxLayout(outputCard);

        auto* outputTitle = new QLabel("Output");
        outputTitle->setObjectName("sectionLabel");

        outputLabel = new QLabel("-");
        outputLabel->setAlignment(Qt::AlignCenter);
        outputLabel->setObjectName("output");

        statusLabel = new QLabel("Ready");
        statusLabel->setAlignment(Qt::AlignCenter);
        statusLabel->setObjectName("status");

        outputLayout->addWidget(outputTitle);
        outputLayout->addStretch();
        outputLayout->addWidget(outputLabel);
        outputLayout->addSpacing(12);
        outputLayout->addWidget(statusLabel);
        outputLayout->addStretch();

        contentLayout->addWidget(outputCard, 1);

        root->addLayout(contentLayout);

        // =========================================================
        // Connections
        // =========================================================
        connect(parseBtn, &QPushButton::clicked,
                this, &MainWindow::parseInput);

        connect(generateBtn, &QPushButton::clicked,
                this, &MainWindow::generateOutput);

        connect(clearBtn, &QPushButton::clicked,
                this, &MainWindow::clearAll);

        connect(swapBtn, &QPushButton::clicked,
                this, &MainWindow::swapOutputToInput);

        connect(inputEdit, &QLineEdit::returnPressed,
                this, &MainWindow::parseInput);

        connect(inputEdit, &QLineEdit::textChanged,
                this, &MainWindow::parseInput);

        // =========================================================
        // Style
        // =========================================================
        setStyleSheet(R"(
            QWidget {
                background: #1e1e1e;
                color: #dddddd;
                font-size: 14px;
                font-family: "Segoe UI";
            }

            QMainWindow {
                background: #1e1e1e;
            }

            QFrame {
                background: #252526;
                border: 1px solid #333;
                border-radius: 12px;
            }

            QLabel#title {
                font-size: 24px;
                font-weight: bold;
                color: white;
            }

            QLabel#subtitle {
                color: #999;
                margin-bottom: 4px;
            }

            QLabel#sectionLabel {
                font-size: 16px;
                font-weight: bold;
                color: white;
                margin-bottom: 6px;
            }

            QLabel#output {
                font-size: 36px;
                font-weight: bold;
                color: #4fc1ff;
            }

            QLabel#status {
                color: #999;
            }

            QLineEdit, QComboBox {
                background: #2d2d30;
                border: 1px solid #444;
                border-radius: 8px;
                padding: 8px;
                min-height: 18px;
            }

            QLineEdit:focus, QComboBox:focus {
                border: 1px solid #4fc1ff;
            }

            QPushButton {
                background: #3a6df0;
                border: none;
                border-radius: 8px;
                padding: 10px 16px;
                font-weight: bold;
            }

            QPushButton:hover {
                background: #4b7dff;
            }

            QPushButton:pressed {
                background: #2f5ed6;
            }
        )");
    }

private:
    QLineEdit* inputEdit = nullptr;

    QLineEdit* phuAmDauEdit = nullptr;
    QLineEdit* amDemEdit = nullptr;
    QLineEdit* amChinhEdit = nullptr;
    QLineEdit* amCuoiEdit = nullptr;

    QComboBox* toneCombo = nullptr;

    QLabel* outputLabel = nullptr;
    QLabel* statusLabel = nullptr;

private:
    static QFrame* createCard() {
        auto* card = new QFrame;
        card->setFrameShape(QFrame::NoFrame);
        return card;
    }

    static Tone toneFromIndex(int idx) {
        switch (idx) {
            case 0: return Tone::NGANG;
            case 1: return Tone::SAC;
            case 2: return Tone::HUYEN;
            case 3: return Tone::HOI;
            case 4: return Tone::NGA;
            case 5: return Tone::NANG;
            default: return Tone::NGANG;
        }
    }

    static int toneToIndex(Tone t) {
        switch (t) {
            case Tone::NGANG: return 0;
            case Tone::SAC:   return 1;
            case Tone::HUYEN: return 2;
            case Tone::HOI:   return 3;
            case Tone::NGA:   return 4;
            case Tone::NANG:  return 5;
        }

        return 0;
    }

    void setStatus(const QString& text, bool ok) {
        statusLabel->setText(text);

        if (ok) {
            statusLabel->setStyleSheet(
                "color:#6dd56d;"
            );

            inputEdit->setStyleSheet(
                "border:1px solid #55aa55;"
            );
        }
        else {
            statusLabel->setStyleSheet(
                "color:#ff7777;"
            );

            inputEdit->setStyleSheet(
                "border:1px solid #cc5555;"
            );
        }
    }

    void parseInput() {
        const auto text = inputEdit->text().trimmed();

        if (text.isEmpty()) {
            return;
        }

        auto result = Syllable::fromUTF8(
            text.toStdString()
        );

        if (!result) {
            setStatus("Invalid syllable", false);
            return;
        }

        const auto& s = *result;

        phuAmDauEdit->setText(
            QString::fromStdString(s.phuAmDau())
        );

        amDemEdit->setText(
            QString::fromStdString(s.amDem())
        );

        amChinhEdit->setText(
            QString::fromStdString(s.amChinh())
        );

        amCuoiEdit->setText(
            QString::fromStdString(s.amCuoi())
        );

        toneCombo->setCurrentIndex(
            toneToIndex(s.tone())
        );

        outputLabel->setText(
            QString::fromStdString(s.toUTF8())
        );

        setStatus("Parsed successfully", true);
    }

    void generateOutput() {
        try {
            Syllable s(
                phuAmDauEdit->text().trimmed().toStdString(),
                amDemEdit->text().trimmed().toStdString(),
                amChinhEdit->text().trimmed().toStdString(),
                amCuoiEdit->text().trimmed().toStdString(),
                toneFromIndex(toneCombo->currentIndex())
            );

            const auto out = s.toUTF8();

            outputLabel->setText(
                QString::fromStdString(out)
            );

            setStatus("Generated successfully", true);
        }
        catch (const std::exception& e) {
            setStatus(
                QString("Error: %1").arg(e.what()),
                false
            );
        }
    }

    void clearAll() {
        inputEdit->clear();

        phuAmDauEdit->clear();
        amDemEdit->clear();
        amChinhEdit->clear();
        amCuoiEdit->clear();

        toneCombo->setCurrentIndex(0);

        outputLabel->setText("-");

        statusLabel->setText("Ready");

        inputEdit->setStyleSheet("");
    }

    void swapOutputToInput() {
        inputEdit->setText(
            outputLabel->text()
        );
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}