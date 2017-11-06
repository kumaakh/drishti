using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;


namespace test_mugshot
{
    public partial class Form1 : Form
    {
        public Form1()
        {
            InitializeComponent();
            MugshotMaker.init();
        }

        private void btnTakePicture_Click(object sender, EventArgs e)
        {
            Task t = new Task(() => {
                Image img = MugshotMaker.takeMugShot(0, 900);
                if (img != null){pbSnap.Image = img;}
            });
            t.Start();
            
        }
    }
}
