/*
 * SPDX-FileCopyrightText: 2019 Boudewijn Rempt <boud@valdyas.org>
 * SPDX-FileCopyrightText: 2020 Agata Cacko <cacko.azh@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KISTAGCHOOSERWIDGET_H
#define KISTAGCHOOSERWIDGET_H

#include <QWidget>
#include "kritaresourcewidgets_export.h"

#include <KisTag.h>
#include <KisTagModel.h>

/**
 * \brief The KisTagChooserWidget class is responsible for all the logic
 *        that the tags combobox has in various resource choosers.
 *
 * It uses KisTagModel as a model for items in the combobox.
 * It is also responsible for the popup for tag removal, renaming and creation
 * that appears on the right side of the tag combobox (via KisTagToolButton)
 * All the logic for adding and removing tags is done through KisTagModel.
 */
class KRITARESOURCEWIDGETS_EXPORT KisTagChooserWidget : public QWidget
{
    Q_OBJECT

public:
    explicit KisTagChooserWidget(KisTagModel* model, QString resourceType, QWidget* parent);
    ~KisTagChooserWidget() override;


    /// \brief setCurrentItem sets the tag from the param as the current tag in the combobox
    /// \param tag tag url to be set as current in the combobox
    /// \return true if successful, false if not successful
    void setCurrentItem(const QString &tag);

    /// \brief currentIndex returns the current index in tags combobox
    /// \return the index of the current item in tags combobox
    int currentIndex() const;

    /// \brief currentlySelectedTag returns the current tag from combobox
    /// \see currentIndex
    /// \return the tag that is currently selected in the tag combobox
    KisTagSP currentlySelectedTag();

    /// \brief selectedTagIsReadOnly checks whether the tag is readonly (generated by Krita)
    /// \return true if the tag was generated by Krita, false if it's just a normal tag
    bool selectedTagIsReadOnly();

    /// \brief update icon files on loading and theme change
    void updateIcons();

Q_SIGNALS:

    /// \brief sigTagChosen is emitted when the selected tag in the combobox changes due to user interaction or by other means
    /// \param tag current tag
    void sigTagChosen(const KisTagSP tag);

public Q_SLOTS:

    /// \brief tagChanged slot for the signal from the combobox that the index changed
    /// \param index new index
    ///
    /// When the index in the combobox changes, for example because of user's interaction,
    ///  combobox emits a signal; this method is called when it happens.
    void tagChanged(int index);

    /// \brief tagToolCreateNewTag slot for the signal from KisTagToolButton that a new tag needs to be created
    /// \param tag tag with the name to be created
    /// \return created tag taken from the model, with a valid id
    void addTag(const QString &tag);
    void addTag(const QString &tag, KoResourceSP resource);
    void addTag(KisTagSP tag, KoResourceSP resource);

private Q_SLOTS:

    /// \brief tagToolRenameCurrentTag slot for the signal from KisTagToolButton that the current tag needs to be renamed
    /// \param newName new name for the tag
    void tagToolRenameCurrentTag(const QString& tag);

    /// \brief tagToolDeleteCurrentTag slot for the signal from the KisTagToolButton that the current tag needs to be deleted
    ///
    /// Note that tags are not deleted but just marked inactive in the database.
    void tagToolDeleteCurrentTag();

    /// \brief tagToolUndeleteLastTag slot for the signal from the KisTagToolButton that the last deleted tag needs to be undeleted
    /// \param tag tag to be undeleted (marked active)
    void tagToolUndeleteLastTag(KisTagSP tag);

    /// \brief tagToolContextMenuAboutToShow slot for the signal from the KisTagToolButton that the popup will be shown soon
    ///
    /// Based on the current tag (if it's readonly or not), the popup looks different, so this function
    ///  sets the correct mode on the KisTagToolButton popup.
    void tagToolContextMenuAboutToShow();

    /// \brief cacheSelectedTag slot that stores current tag selection.
    ///
    /// Used to allow restoration of tag even after a model reset. Will store
    /// the tag just before model resets.
    void cacheSelectedTag();

    /// \brief restoreTagFromCache slot designed to restore a selected tag from previously cached selection.
    ///
    /// Companion to `cacheSelectedTag`, this method restore the selection after model reset.
    void restoreTagFromCache();

    void slotTagModelDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> roles);

private:


    /// \brief setCurrentIndex sets the current index in the combobox
    /// \param index index is the index of the tag in the combobox
    void setCurrentIndex(int index);

    enum OverwriteDialogOptions {
        Replace,
        Undelete,
        Cancel
    };

    OverwriteDialogOptions overwriteTagDialog(KisTagChooserWidget* parent, bool undelete);

private:
    class Private;
    Private* const d;

};

#endif // KISTAGCHOOSERWIDGET_H
