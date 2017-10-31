using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace test_mugshot
{
    class MugshotMaker
    {
        [DllImport("drishti-mugshot.dll",
           CallingConvention = CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.I4)]
        private static extern int takeMugShot(int camIdx, int width, int height, [MarshalAs(UnmanagedType.LPArray)] byte[] outBuff, int maxFrames);
        public static Image takeMugShot(int camIdx, int maxFrames)
        {
            int width = 480, height = 640; // thats all we support in native
            byte[] buff = new byte[3 * width * height];
            int rc=takeMugShot(camIdx, width, height, buff, maxFrames);
            if(rc==0)
            {
                Bitmap bmp = new Bitmap(width, height, PixelFormat.Format24bppRgb);
                Rectangle dimension = new Rectangle(0, 0, bmp.Width, bmp.Height);
                BitmapData picData = bmp.LockBits(dimension, ImageLockMode.ReadWrite, bmp.PixelFormat);
                IntPtr pixelStartAddress = picData.Scan0;
                Marshal.Copy(buff, 0, pixelStartAddress, buff.Length);
                bmp.UnlockBits(picData);
                return bmp;
            }
            return null;

        }
        public static void init()
        {
            takeMugShot(0,0,0,null,0);
        }
    }
}
