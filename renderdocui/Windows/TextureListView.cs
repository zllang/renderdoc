/******************************************************************************
 * The MIT License (MIT)
 * 
 * Copyright (c) 2015-2017 Baldur Karlsson
 * Copyright (c) 2014 Crytek
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 ******************************************************************************/


using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using renderdoc;
using renderdocui.Code;

namespace renderdocui.Controls
{
    public partial class TextureListView : ListView
    {
        private TextureListViewComparer m_Sort = new TextureListViewComparer();
        private static readonly object GoIconClickEvent = new object();
        public event EventHandler<GoIconClickEventArgs> GoIconClick
        {
            add { Events.AddHandler(GoIconClickEvent, value); }
            remove { Events.RemoveHandler(GoIconClickEvent, value); }
        }
        protected virtual void OnGoIconClick(GoIconClickEventArgs e)
        {
            EventHandler<GoIconClickEventArgs> handler = (EventHandler<GoIconClickEventArgs>)Events[GoIconClickEvent];
            if (handler != null)
                handler(this, e);
        }

        public Core m_Core = null;

        public TextureListView()
        {
            View = View.Details;
            GridLines = true;
            Sorting = SortOrder.None;
            FullRowSelect = true;
            ColumnHeader header = new ColumnHeader();
            header.Text = "Name";
            Columns.Add(header);
            header = new ColumnHeader();
            header.Text = "Width";
            header.Tag = "Numeric";
            Columns.Add(header);
            header = new ColumnHeader();
            header.Text = "Height";
            header.Tag = "Numeric";
            Columns.Add(header);
            header = new ColumnHeader();
            header.Text = "Format";
            Columns.Add(header);
            header = new ColumnHeader();
            header.Text = "ResType";
            Columns.Add(header);
            header = new ColumnHeader();
            header.Text = "ID";
            Columns.Add(header);

            ListViewItemSorter = m_Sort;
        }

        protected override void OnSelectedIndexChanged(EventArgs e)
        {
            base.OnSelectedIndexChanged(e);
            if (this.SelectedItems.Count > 0)
            {
                OnGoIconClick(new GoIconClickEventArgs(new ResourceId(ulong.Parse(this.SelectedItems[0].SubItems[5].Text))));
            }
        }

        protected override void OnVisibleChanged(EventArgs e)
        {
            base.OnVisibleChanged(e);

            if (Visible)
            {
                FillTextureList("", true, true);
            }
        }

        protected override void OnColumnClick(ColumnClickEventArgs e)
        {
            base.OnColumnClick(e);
            m_Sort.m_Column = e.Column;
            m_Sort.m_Order  = (m_Sort.m_Order == SortOrder.Ascending )? SortOrder.Descending : SortOrder.Ascending;
            Sort();
        }
        public void FillTextureList(string filter, bool RTs, bool Texs)
        {
            Items.Clear();

            if (m_Core == null || !m_Core.LogLoaded)
            {
                return;
            }

            for (int i = 0; i < m_Core.CurTextures.Length; i++)
            {
                bool include = false;
                include |= (RTs && ((m_Core.CurTextures[i].creationFlags & TextureCreationFlags.RTV) > 0 ||
                                    (m_Core.CurTextures[i].creationFlags & TextureCreationFlags.DSV) > 0));
                include |= (Texs && (m_Core.CurTextures[i].creationFlags & TextureCreationFlags.RTV) == 0 &&
                                    (m_Core.CurTextures[i].creationFlags & TextureCreationFlags.DSV) == 0);
                include |= (filter.Length > 0 && (m_Core.CurTextures[i].name.ToUpperInvariant().Contains(filter.ToUpperInvariant())));
                include |= (!RTs && !Texs && filter.Length == 0);

                if (include)
                {
                    ListViewItem item = new ListViewItem();
                    item.Text = m_Core.CurTextures[i].name;
                    item.SubItems.Add(String.Format("{0}", m_Core.CurTextures[i].width));
                    item.SubItems.Add(String.Format("{0}", m_Core.CurTextures[i].height));
                    item.SubItems.Add(m_Core.CurTextures[i].format.strname);
                    item.SubItems.Add(m_Core.CurTextures[i].resType.ToString());
                    item.SubItems.Add(String.Format("{0}", m_Core.CurTextures[i].ID));

                    Items.Add(item);
                }
            }
        }
    }
    public class TextureListViewComparer: System.Collections.IComparer
    {
        public int m_Column = 0;
        public System.Windows.Forms.SortOrder m_Order = SortOrder.Ascending; 
        public int Compare(object x, object y)
        {
            ListViewItem lx = (ListViewItem)x;
            ListViewItem ly = (ListViewItem)y;
            if (lx.ListView.Columns[m_Column].Tag == null)
            {
                lx.ListView.Columns[m_Column].Tag = "Text";
            }
            int result = 0;
            if (lx.ListView.Columns[m_Column].Tag.ToString() == "Numeric")
            {
                long xv = long.Parse(lx.SubItems[m_Column].Text);
                long yv = long.Parse(ly.SubItems[m_Column].Text);
                if (m_Order == SortOrder.Ascending)
                {
                    result = xv.CompareTo(yv);
                }
                else
                {
                    result = yv.CompareTo(xv);
                }
            }
            else
            {
                string xv = lx.SubItems[m_Column].Text;
                string yv = ly.SubItems[m_Column].Text;
                if (m_Order == SortOrder.Ascending)
                {
                    result = xv.CompareTo(yv);
                }
                else
                {
                    result = yv.CompareTo(xv);
                }
            }
            return result;
        }
    }
}

