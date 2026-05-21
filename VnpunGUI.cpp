// VnpunGUI.cpp
// Single-file Qt Widgets GUI for vnpun.
// Put this file next to Syllable.h / Syllable.cpp.
//
// UX contract:
// - Origin and Transforming behave like familiar line edits.
// - Ctrl + drag  -> move whole syllable.
// - Alt + drag   -> move tone / dấu thanh.
// - Normal drag  -> move component/radical.
// - UI component level: Initial and Rhyme.
// - Rhyme = medial + nucleus + final.

#include "Syllable.h"

// Important: include Qt after Syllable.h.
// Syllable.h includes broad STL headers; Qt defines macro `emit`.
// If Qt is included first, GCC's std::syncstream::emit can be macro-broken.
#include <QtWidgets>

#include <functional>
#include <optional>
#include <string>
#include <utility>

namespace {

QString qs(const std::string& s) {
    return QString::fromUtf8(s.data(), int(s.size()));
}

std::string ss(const QString& s) {
    return s.toUtf8().toStdString();
}

QString toneText(vnpun::Tone tone) {
    switch (tone) {
    case vnpun::Tone::NGANG: return QStringLiteral("ngang");
    case vnpun::Tone::SAC:   return QStringLiteral("sắc");
    case vnpun::Tone::HUYEN: return QStringLiteral("huyền");
    case vnpun::Tone::HOI:   return QStringLiteral("hỏi");
    case vnpun::Tone::NGA:   return QStringLiteral("ngã");
    case vnpun::Tone::NANG:  return QStringLiteral("nặng");
    }
    return QStringLiteral("unknown");
}

struct RadicalParts {
    QString initial;
    QString medial;
    QString nucleus;
    QString final;
    vnpun::Tone tone = vnpun::Tone::NGANG;
};

struct RhymeParts {
    QString medial;
    QString nucleus;
    QString final;
};

QString rhymeText(const RadicalParts& p) {
    return p.medial + p.nucleus + p.final;
}

struct SyllableItem {
    QString id;
    QString raw;
    std::optional<vnpun::Syllable> parsed;
};

std::optional<vnpun::Syllable> parseOne(const QString& raw) {
    try {
        return vnpun::Syllable::fromUTF8(ss(raw));
    } catch (...) {
        return std::nullopt;
    }
}

SyllableItem makeItem(const QString& id, const QString& raw) {
    return SyllableItem{ id, raw, parseOne(raw) };
}

RadicalParts partsOf(const SyllableItem& item) {
    RadicalParts p;
    if (!item.parsed) return p;

    p.initial = qs(item.parsed->phuAmDau());
    p.medial  = qs(item.parsed->amDem());
    p.nucleus = qs(item.parsed->amChinh());
    p.final   = qs(item.parsed->amCuoi());
    p.tone    = item.parsed->tone();
    return p;
}

RhymeParts rhymeOf(const SyllableItem& item) {
    const RadicalParts p = partsOf(item);
    return RhymeParts{ p.medial, p.nucleus, p.final };
}

QString joinRaw(const QVector<SyllableItem>& items) {
    QStringList out;
    out.reserve(items.size());
    for (const auto& item : items) out << item.raw;
    return out.join(QChar(' '));
}

QStringList splitSyllables(const QString& text) {
    return text.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
}

QString nextId() {
    static qsizetype counter = 1;
    return QStringLiteral("s%1").arg(counter++);
}

enum class FieldId {
    Origin,
    Transforming
};

enum class Aspect {
    Initial = 0,
    Rhyme   = 1,
    Tone    = 2
};

QString fieldName(FieldId f) {
    return f == FieldId::Origin ? QStringLiteral("Origin") : QStringLiteral("Transforming");
}

QString aspectName(Aspect a) {
    switch (a) {
    case Aspect::Initial: return QStringLiteral("initial");
    case Aspect::Rhyme:   return QStringLiteral("rhyme");
    case Aspect::Tone:    return QStringLiteral("tone");
    }
    return QStringLiteral("unknown");
}

QVector<Aspect> allAspects() {
    return {
        Aspect::Initial,
        Aspect::Rhyme,
        Aspect::Tone
    };
}

QString radicalValue(const SyllableItem& item, Aspect aspect) {
    const RadicalParts p = partsOf(item);

    switch (aspect) {
    case Aspect::Initial: return p.initial;
    case Aspect::Rhyme:   return rhymeText(p);
    case Aspect::Tone:    return toneText(p.tone);
    }

    return {};
}

std::optional<vnpun::Syllable> rebuildSyllable(
    const QString& initial,
    const QString& medial,
    const QString& nucleus,
    const QString& final,
    vnpun::Tone tone
) {
    try {
        return vnpun::Syllable(
            ss(initial),
            ss(medial),
            ss(nucleus),
            ss(final),
            tone
        );
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<vnpun::Syllable> rebuildWithInitial(
    const SyllableItem& base,
    const QString& newInitial
) {
    if (!base.parsed) return std::nullopt;
    const RadicalParts p = partsOf(base);
    return rebuildSyllable(newInitial, p.medial, p.nucleus, p.final, p.tone);
}

std::optional<vnpun::Syllable> rebuildWithRhyme(
    const SyllableItem& base,
    const RhymeParts& newRhyme
) {
    if (!base.parsed) return std::nullopt;
    const RadicalParts p = partsOf(base);
    return rebuildSyllable(p.initial, newRhyme.medial, newRhyme.nucleus, newRhyme.final, p.tone);
}

std::optional<vnpun::Syllable> rebuildWithTone(
    const SyllableItem& base,
    vnpun::Tone newTone
) {
    if (!base.parsed) return std::nullopt;
    const RadicalParts p = partsOf(base);
    return rebuildSyllable(p.initial, p.medial, p.nucleus, p.final, newTone);
}

template <typename T>
void moveItem(QVector<T>& v, int from, int to) {
    if (from < 0 || from >= v.size()) return;
    if (to < 0 || to >= v.size()) return;
    if (from == to) return;

    T item = v.takeAt(from);
    v.insert(to, item);
}

} // namespace

class SyllableLineEdit final : public QLineEdit {
public:
    using FocusCallback = std::function<void(FieldId, int)>;
    using SyllableMoveCallback = std::function<void(FieldId, int, int)>;
    using ComponentMoveCallback = std::function<void(FieldId, Aspect, int, int)>;

    explicit SyllableLineEdit(FieldId field, QWidget* parent = nullptr)
        : QLineEdit(parent), field_(field) {
        setAcceptDrops(true);
        setDragEnabled(false);
        setClearButtonEnabled(false);

        setMinimumHeight(78);
        QFont f = font();
        f.setPointSize(qMax(14, f.pointSize() + 2));
        setFont(f);
        setTextMargins(12, 10, 12, 10);

        setPlaceholderText(field_ == FieldId::Origin
            ? QStringLiteral("Nhập Origin...")
            : QStringLiteral("Transforming..."));

        setToolTip(QStringLiteral(
            "Kéo thường: di chuyển initial/rhyme\n"
            "Ctrl + kéo: di chuyển cả syllable\n"
            "Alt + kéo: di chuyển dấu thanh/tone\n"
            "Rhyme = âm đệm + âm chính + âm cuối"
        ));

        connect(this, &QLineEdit::textChanged, this, [this] {
            rebuildRanges();
        });

        connect(this, &QLineEdit::cursorPositionChanged, this, [this](int, int now) {
            if (onFocused) onFocused(field_, indexAtChar(now));
        });
    }

    void setItems(const QVector<SyllableItem>& items) {
        items_ = items;
        rebuildRanges();
    }

    FocusCallback onFocused;
    SyllableMoveCallback onSyllableMove;
    ComponentMoveCallback onComponentMove;

protected:
    void mousePressEvent(QMouseEvent* e) override {
        pressPos_ = e->pos();
        pressClock_.restart();

        pressIndex_ = indexAtPoint(e->pos());

        if (e->modifiers().testFlag(Qt::AltModifier)) {
            pressAspect_ = Aspect::Tone;
        } else {
            pressAspect_ = aspectAtPoint(e->pos());
        }

        possibleDrag_ = pressIndex_ >= 0;

        QLineEdit::mousePressEvent(e);

        if (pressIndex_ >= 0 && onFocused) {
            onFocused(field_, pressIndex_);
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (!(e->buttons() & Qt::LeftButton) || !possibleDrag_ || pressIndex_ < 0) {
            QLineEdit::mouseMoveEvent(e);
            return;
        }

        const int dist = (e->pos() - pressPos_).manhattanLength();
        if (dist < QApplication::startDragDistance()) {
            QLineEdit::mouseMoveEvent(e);
            return;
        }

        const bool syllableMode = e->modifiers().testFlag(Qt::ControlModifier);
        const bool toneMode = !syllableMode && e->modifiers().testFlag(Qt::AltModifier);

        // Priority:
        // Ctrl + drag = syllable
        // Alt + drag  = tone
        // drag        = initial/rhyme
        if (toneMode) {
            pressAspect_ = Aspect::Tone;
        }

        // Ctrl+drag and Alt+drag start immediately.
        // Normal component drag starts after a short hold, so quick mouse drag can still select text.
        if (!syllableMode && !toneMode && pressClock_.elapsed() < 180) {
            QLineEdit::mouseMoveEvent(e);
            return;
        }

        startVnPunDrag(syllableMode);
    }

    void mouseReleaseEvent(QMouseEvent* e) override {
        possibleDrag_ = false;
        pressIndex_ = -1;
        QLineEdit::mouseReleaseEvent(e);
    }

    void dragEnterEvent(QDragEnterEvent* e) override {
        if (e->mimeData()->hasFormat(QStringLiteral("application/vnd.vnpun.dnd"))) {
            e->acceptProposedAction();
            return;
        }
        QLineEdit::dragEnterEvent(e);
    }

    void dragMoveEvent(QDragMoveEvent* e) override {
        if (e->mimeData()->hasFormat(QStringLiteral("application/vnd.vnpun.dnd"))) {
            const int slot = dropSlotAtPoint(e->position().toPoint());
            showDropCaret(slot);
            e->acceptProposedAction();
            return;
        }
        QLineEdit::dragMoveEvent(e);
    }

    void dragLeaveEvent(QDragLeaveEvent* e) override {
        Q_UNUSED(e);
        setCursorPosition(cursorPosition());
    }

    void dropEvent(QDropEvent* e) override {
        const auto fmt = QStringLiteral("application/vnd.vnpun.dnd");

        if (!e->mimeData()->hasFormat(fmt)) {
            QLineEdit::dropEvent(e);
            return;
        }

        QByteArray payload = e->mimeData()->data(fmt);
        QDataStream in(&payload, QIODevice::ReadOnly);

        qint32 sourceFieldInt = 0;
        qint32 from = -1;
        qint32 isSyllable = 0;
        qint32 aspectInt = 0;

        in >> sourceFieldInt >> from >> isSyllable >> aspectInt;

        const FieldId sourceField = sourceFieldInt == 0
            ? FieldId::Origin
            : FieldId::Transforming;

        if (sourceField != field_) {
            e->ignore();
            return;
        }

        if (items_.isEmpty()) {
            e->ignore();
            return;
        }

        const int slot = dropSlotAtPoint(e->position().toPoint());
        const int to = indexFromDropSlot(slot, int(from), items_.size());

        if (to < 0 || to >= items_.size() || from < 0 || from >= items_.size()) {
            e->ignore();
            return;
        }

        if (to == from) {
            e->acceptProposedAction();
            return;
        }

        if (isSyllable) {
            if (onSyllableMove) {
                onSyllableMove(field_, int(from), to);
            }
        } else {
            const Aspect aspect = Aspect(aspectInt);
            if (onComponentMove) {
                onComponentMove(field_, aspect, int(from), to);
            }
        }

        possibleDrag_ = false;
        pressIndex_ = -1;
        e->acceptProposedAction();
    }

private:
    struct Range {
        int start = 0;
        int end = 0;
    };

    void startVnPunDrag(bool syllableMode) {
        if (pressIndex_ < 0 || pressIndex_ >= items_.size()) return;

        possibleDrag_ = false;

        QDrag* drag = new QDrag(this);
        QMimeData* mime = new QMimeData;

        QByteArray payload;
        QDataStream out(&payload, QIODevice::WriteOnly);

        out << qint32(field_ == FieldId::Origin ? 0 : 1)
            << qint32(pressIndex_)
            << qint32(syllableMode ? 1 : 0)
            << qint32(static_cast<int>(pressAspect_));

        mime->setData(QStringLiteral("application/vnd.vnpun.dnd"), payload);
        drag->setMimeData(mime);

        drag->setPixmap(makeDragPixmap(syllableMode));
        drag->setHotSpot(QPoint(12, 14));
        drag->exec(Qt::MoveAction);
    }

    QPixmap makeDragPixmap(bool syllableMode) const {
        QString label;

        if (syllableMode) {
            label = QStringLiteral("syllable: %1").arg(items_[pressIndex_].raw);
        } else {
            label = QStringLiteral("%1: %2")
                .arg(aspectName(pressAspect_))
                .arg(radicalValue(items_[pressIndex_], pressAspect_));
        }

        QFontMetrics fm(font());
        const int w = qMax(180, fm.horizontalAdvance(label) + 32);
        const int h = 38;

        QPixmap pixmap(w, h);
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setPen(QPen(Qt::darkGray));
        painter.setBrush(QColor(245, 245, 245));
        painter.drawRoundedRect(QRect(0, 0, w - 1, h - 1), 9, 9);
        painter.drawText(QRect(14, 0, w - 28, h), Qt::AlignVCenter | Qt::AlignLeft, label);
        painter.end();

        return pixmap;
    }

    void rebuildRanges() {
        ranges_.clear();

        const QString t = text();
        QRegularExpression re(QStringLiteral("\\S+"));
        auto it = re.globalMatch(t);

        while (it.hasNext()) {
            const auto m = it.next();
            ranges_.push_back({
                int(m.capturedStart()),
                int(m.capturedEnd())
            });
        }
    }

    int indexAtChar(int charIndex) const {
        if (ranges_.isEmpty()) return -1;

        for (int i = 0; i < ranges_.size(); ++i) {
            if (charIndex >= ranges_[i].start && charIndex <= ranges_[i].end) {
                return i;
            }
        }

        if (charIndex < ranges_.front().start) return 0;
        return ranges_.size() - 1;
    }

    int indexAtPoint(const QPoint& p) {
        return indexAtChar(cursorPositionAt(p));
    }

    int dropSlotAtPoint(const QPoint& p) {
        if (ranges_.isEmpty()) return 0;

        const int charPos = cursorPositionAt(p);

        for (int i = 0; i < ranges_.size(); ++i) {
            const int start = ranges_[i].start;
            const int end = ranges_[i].end;
            const int mid = start + (end - start) / 2;

            if (charPos < start) return i;

            if (charPos >= start && charPos <= end) {
                return charPos <= mid ? i : i + 1;
            }
        }

        return ranges_.size();
    }

    int indexFromDropSlot(int slot, int from, int count) const {
        if (count <= 0) return -1;

        slot = qBound(0, slot, count);

        int to = slot;

        // slot is insertion boundary:
        // 0 = before first syllable
        // count = after last syllable
        //
        // moveItem expects final item index after removing source item.
        if (slot > from) {
            to = slot - 1;
        }

        return qBound(0, to, count - 1);
    }

    void showDropCaret(int slot) {
        if (ranges_.isEmpty()) return;

        slot = qBound(0, slot, ranges_.size());

        if (slot == ranges_.size()) {
            setCursorPosition(ranges_.last().end);
        } else {
            setCursorPosition(ranges_[slot].start);
        }
    }

    Aspect aspectAtPoint(const QPoint& p) {
        const int idx = indexAtPoint(p);

        if (idx < 0 || idx >= ranges_.size() || idx >= items_.size()) {
            return Aspect::Rhyme;
        }

        const QString leftText = text().left(ranges_[idx].start);
        const QString token = text().mid(
            ranges_[idx].start,
            ranges_[idx].end - ranges_[idx].start
        );

        const RadicalParts parts = partsOf(items_[idx]);
        const QString initial = parts.initial;

        const QFontMetrics fm(font());
        const int tokenLeft = textMargins().left() + fm.horizontalAdvance(leftText);
        const int tokenWidth = qMax(1, fm.horizontalAdvance(token));
        const int initialWidth = qMax(0, fm.horizontalAdvance(initial));

        const int rel = qBound(0, p.x() - tokenLeft, tokenWidth - 1);

        if (!initial.isEmpty() && rel <= initialWidth + 3) {
            return Aspect::Initial;
        }

        return Aspect::Rhyme;
    }

private:
    FieldId field_;
    QVector<SyllableItem> items_;
    QVector<Range> ranges_;

    QPoint pressPos_;
    QElapsedTimer pressClock_;

    int pressIndex_ = -1;
    Aspect pressAspect_ = Aspect::Rhyme;
    bool possibleDrag_ = false;
};

class DetailsPanel final : public QTextEdit {
public:
    explicit DetailsPanel(QWidget* parent = nullptr) : QTextEdit(parent) {
        setReadOnly(true);
        setMinimumWidth(300);
        setText(QStringLiteral("Radicals / Details\n\nChọn một syllable để xem chi tiết."));
    }

    void showItem(FieldId field, const SyllableItem* item) {
        if (!item) {
            setText(QStringLiteral("Radicals / Details\n\nKhông có syllable được chọn."));
            return;
        }

        if (!item->parsed) {
            setText(QStringLiteral(
                "Radicals / Details\n\n"
                "%1: %2\n\n"
                "Unparsed"
            ).arg(fieldName(field), item->raw));
            return;
        }

        const RadicalParts p = partsOf(*item);

        setText(QStringLiteral(
            "Radicals / Details\n\n"
            "%1: %2\n\n"
            "Phụ âm đầu: %3\n"
            "Âm đệm:     %4\n"
            "Âm chính:   %5\n"
            "Âm cuối:    %6\n"
            "Phần vần:   %7\n"
            "Thanh điệu: %8\n"
            "Raw:        %9\n"
            "toUTF8:     %10"
        ).arg(
            fieldName(field),
            item->raw,
            p.initial,
            p.medial,
            p.nucleus,
            p.final,
            rhymeText(p),
            toneText(p.tone),
            item->raw,
            qs(item->parsed->toUTF8())
        ));
    }
};

class MainWindow final : public QMainWindow {
public:
    MainWindow() {
        setWindowTitle(QStringLiteral("vnpunGUI"));
        resize(1100, 620);

        originEdit_ = new SyllableLineEdit(FieldId::Origin);
        transformingEdit_ = new SyllableLineEdit(FieldId::Transforming);

        details_ = new DetailsPanel;

        overview_ = new QTableWidget;
        overview_->setEditTriggers(QAbstractItemView::NoEditTriggers);
        overview_->setSelectionMode(QAbstractItemView::SingleSelection);
        overview_->setSelectionBehavior(QAbstractItemView::SelectItems);
        overview_->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        overview_->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

        auto* left = new QWidget;
        auto* leftLayout = new QVBoxLayout(left);
        leftLayout->setContentsMargins(16, 16, 16, 16);
        leftLayout->setSpacing(10);

        leftLayout->addWidget(makeLabel(QStringLiteral("Origin")));
        leftLayout->addWidget(originEdit_);
        leftLayout->addSpacing(22);
        leftLayout->addWidget(makeLabel(QStringLiteral("Transforming")));
        leftLayout->addWidget(transformingEdit_);
        leftLayout->addStretch(1);

        auto* right = new QWidget;
        auto* rightLayout = new QVBoxLayout(right);
        rightLayout->setContentsMargins(16, 16, 16, 16);
        rightLayout->setSpacing(10);

        rightLayout->addWidget(details_, 2);
        rightLayout->addWidget(makeLabel(QStringLiteral("Origin Radicals Overview")));
        rightLayout->addWidget(overview_, 3);

        auto* splitter = new QSplitter(Qt::Horizontal);
        splitter->addWidget(left);
        splitter->addWidget(right);
        splitter->setStretchFactor(0, 3);
        splitter->setStretchFactor(1, 2);
        setCentralWidget(splitter);

        connect(originEdit_, &QLineEdit::textEdited, this, [this](const QString& text) {
            onOriginEdited(text);
        });

        connect(transformingEdit_, &QLineEdit::textEdited, this, [this](const QString& text) {
            onTransformingEdited(text);
        });

        originEdit_->onFocused = [this](FieldId field, int index) {
            onSyllableFocused(field, index);
        };

        transformingEdit_->onFocused = [this](FieldId field, int index) {
            onSyllableFocused(field, index);
        };

        originEdit_->onSyllableMove = [this](FieldId field, int from, int to) {
            moveSyllable(field, from, to);
        };

        transformingEdit_->onSyllableMove = [this](FieldId field, int from, int to) {
            moveSyllable(field, from, to);
        };

        originEdit_->onComponentMove = [this](FieldId field, Aspect aspect, int from, int to) {
            moveComponent(field, aspect, from, to);
        };

        transformingEdit_->onComponentMove = [this](FieldId field, Aspect aspect, int from, int to) {
            moveComponent(field, aspect, from, to);
        };

        connect(overview_, &QTableWidget::cellClicked, this, [this](int, int col) {
            if (col >= 0 && col < originItems_.size()) {
                details_->showItem(FieldId::Origin, &originItems_[col]);
                originEdit_->setFocus();
            }
        });

        originItems_ = {
            makeItem(nextId(), QStringLiteral("mẹ")),
            makeItem(nextId(), QStringLiteral("bé")),
            makeItem(nextId(), QStringLiteral("khỏe"))
        };

        transformingItems_ = {
            makeItem(originItems_[0].id, QStringLiteral("mẹ")),
            makeItem(originItems_[1].id, QStringLiteral("bé")),
            makeItem(originItems_[2].id, QStringLiteral("khỏe"))
        };

        refreshAll();
    }

private:
    QLabel* makeLabel(const QString& text) const {
        auto* l = new QLabel(text);
        QFont f = l->font();
        f.setBold(true);
        f.setPointSize(qMax(13, f.pointSize() + 1));
        l->setFont(f);
        return l;
    }

    void onOriginEdited(const QString& text) {
        if (updating_) return;

        const QStringList newTokens = splitSyllables(text);
        syncOriginText(newTokens);
        refreshAllPreservingTextInput(FieldId::Origin);
    }

    void onTransformingEdited(const QString& text) {
        if (updating_) return;

        const QStringList tokens = splitSyllables(text);
        QVector<SyllableItem> next;
        next.reserve(tokens.size());

        for (int i = 0; i < tokens.size(); ++i) {
            const QString id = i < transformingItems_.size()
                ? transformingItems_[i].id
                : (i < originItems_.size() ? originItems_[i].id : nextId());

            next.push_back(makeItem(id, tokens[i]));
        }

        transformingItems_ = next;
        refreshAllPreservingTextInput(FieldId::Transforming);
    }

    void onSyllableFocused(FieldId field, int index) {
        const QVector<SyllableItem>& items = field == FieldId::Origin
            ? originItems_
            : transformingItems_;

        if (index < 0 || index >= items.size()) {
            details_->showItem(field, nullptr);
            return;
        }

        details_->showItem(field, &items[index]);
    }

    void moveSyllable(FieldId field, int from, int to) {
        Q_UNUSED(field);

        if (from == to) return;
        if (from < 0 || to < 0) return;

        // Contract: Origin/Transforming keep mapping by index.
        // Therefore syllable reorder in either field reorders the pair.
        if (from >= originItems_.size() || to >= originItems_.size()) return;
        if (from >= transformingItems_.size() || to >= transformingItems_.size()) return;

        moveItem(originItems_, from, to);
        moveItem(transformingItems_, from, to);

        refreshAll();
    }

    void moveComponent(FieldId field, Aspect aspect, int from, int to) {
        QVector<SyllableItem>& items = field == FieldId::Origin
            ? originItems_
            : transformingItems_;

        if (from < 0 || from >= items.size() || to < 0 || to >= items.size() || from == to) {
            return;
        }

        QVector<SyllableItem> next = items;

        if (aspect == Aspect::Initial) {
            QVector<QString> initials;
            initials.reserve(items.size());

            for (const auto& item : items) {
                initials.push_back(partsOf(item).initial);
            }

            moveItem(initials, from, to);

            for (int i = 0; i < next.size(); ++i) {
                const auto rebuilt = rebuildWithInitial(items[i], initials[i]);
                if (!rebuilt) continue;

                next[i].parsed = rebuilt;
                next[i].raw = qs(rebuilt->toUTF8());
            }
        }

        if (aspect == Aspect::Rhyme) {
            QVector<RhymeParts> rhymes;
            rhymes.reserve(items.size());

            for (const auto& item : items) {
                rhymes.push_back(rhymeOf(item));
            }

            moveItem(rhymes, from, to);

            for (int i = 0; i < next.size(); ++i) {
                const auto rebuilt = rebuildWithRhyme(items[i], rhymes[i]);
                if (!rebuilt) continue;

                next[i].parsed = rebuilt;
                next[i].raw = qs(rebuilt->toUTF8());
            }
        }

        if (aspect == Aspect::Tone) {
            QVector<vnpun::Tone> tones;
            tones.reserve(items.size());

            for (const auto& item : items) {
                tones.push_back(partsOf(item).tone);
            }

            moveItem(tones, from, to);

            for (int i = 0; i < next.size(); ++i) {
                const auto rebuilt = rebuildWithTone(items[i], tones[i]);
                if (!rebuilt) continue;

                next[i].parsed = rebuilt;
                next[i].raw = qs(rebuilt->toUTF8());
            }
        }

        items = next;
        refreshAll();
    }

    void syncOriginText(const QStringList& newTokens) {
        const int oldN = originItems_.size();
        const int newN = newTokens.size();

        int prefix = 0;
        while (prefix < oldN && prefix < newN && originItems_[prefix].raw == newTokens[prefix]) {
            ++prefix;
        }

        int oldSuffix = oldN - 1;
        int newSuffix = newN - 1;

        while (oldSuffix >= prefix && newSuffix >= prefix &&
               originItems_[oldSuffix].raw == newTokens[newSuffix]) {
            --oldSuffix;
            --newSuffix;
        }

        const int deleteCount = oldSuffix - prefix + 1;
        if (deleteCount > 0) {
            for (int i = 0; i < deleteCount; ++i) {
                originItems_.removeAt(prefix);
                if (prefix < transformingItems_.size()) {
                    transformingItems_.removeAt(prefix);
                }
            }
        }

        const int insertCount = newSuffix - prefix + 1;
        for (int i = 0; i < insertCount; ++i) {
            const QString raw = newTokens[prefix + i];
            const QString id = nextId();

            originItems_.insert(prefix + i, makeItem(id, raw));
            transformingItems_.insert(prefix + i, makeItem(id, raw));
        }

        for (int i = 0; i < originItems_.size() && i < newTokens.size(); ++i) {
            if (originItems_[i].raw != newTokens[i]) {
                originItems_[i] = makeItem(originItems_[i].id, newTokens[i]);
            }
        }
    }

    void refreshAllPreservingTextInput(FieldId editedField) {
        QSignalBlocker b1(originEdit_);
        QSignalBlocker b2(transformingEdit_);
        updating_ = true;

        if (editedField != FieldId::Origin) {
            originEdit_->setText(joinRaw(originItems_));
        }

        if (editedField != FieldId::Transforming) {
            transformingEdit_->setText(joinRaw(transformingItems_));
        }

        originEdit_->setItems(originItems_);
        transformingEdit_->setItems(transformingItems_);
        rebuildOverview();

        updating_ = false;
    }

    void refreshAll() {
        QSignalBlocker b1(originEdit_);
        QSignalBlocker b2(transformingEdit_);
        updating_ = true;

        originEdit_->setText(joinRaw(originItems_));
        transformingEdit_->setText(joinRaw(transformingItems_));

        originEdit_->setItems(originItems_);
        transformingEdit_->setItems(transformingItems_);

        rebuildOverview();

        updating_ = false;
    }

    void rebuildOverview() {
        const QVector<Aspect> aspects = allAspects();

        overview_->clear();
        overview_->setRowCount(aspects.size());
        overview_->setColumnCount(originItems_.size());

        QStringList rowLabels;
        for (Aspect a : aspects) {
            rowLabels << aspectName(a);
        }
        overview_->setVerticalHeaderLabels(rowLabels);

        QStringList colLabels;
        for (int i = 0; i < originItems_.size(); ++i) {
            colLabels << QString::number(i) + QStringLiteral("\n") + originItems_[i].raw;
        }
        overview_->setHorizontalHeaderLabels(colLabels);

        for (int row = 0; row < aspects.size(); ++row) {
            for (int col = 0; col < originItems_.size(); ++col) {
                const QString value = radicalValue(originItems_[col], aspects[row]);
                auto* cell = new QTableWidgetItem(value.isEmpty() ? QStringLiteral(" ") : value);
                cell->setTextAlignment(Qt::AlignCenter);
                overview_->setItem(row, col, cell);
            }
        }

        overview_->resizeColumnsToContents();
        overview_->resizeRowsToContents();
    }

private:
    SyllableLineEdit* originEdit_ = nullptr;
    SyllableLineEdit* transformingEdit_ = nullptr;
    DetailsPanel* details_ = nullptr;
    QTableWidget* overview_ = nullptr;

    QVector<SyllableItem> originItems_;
    QVector<SyllableItem> transformingItems_;

    bool updating_ = false;
};

int main(int argc, char** argv) {
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}
