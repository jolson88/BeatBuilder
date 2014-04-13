using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;
using Windows.Storage.Streams;

namespace BeatBuilder.Audio
{
    [ComImport]
    [Guid("905a0fef-bc53-11df-8c49-001e4fc686da")]
    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    internal interface IBufferByteAccess
    {
        unsafe byte* Buffer
        {
            get;
        }
    }

    public unsafe class FloatBuffer
    {
        private IBuffer m_buffer;
        private float* m_dataPtr;

        public uint Length
        {
            get { return m_buffer.Capacity / sizeof(float); }
        }

        public float this[int key]
        {
            get
            {
                if (key > this.Length)
                    throw new IndexOutOfRangeException();

                return m_dataPtr[key];
            }
            set
            {
                if (key > this.Length)
                    throw new IndexOutOfRangeException();

                m_dataPtr[key] = value;
            }
        }

        public FloatBuffer(IBuffer buffer)
        {
            m_buffer = buffer;
            m_dataPtr = (float*)((IBufferByteAccess)buffer).Buffer;
        }
    }
}
