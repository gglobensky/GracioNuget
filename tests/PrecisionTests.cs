using System;
using Xunit;
using Gracio;

namespace Gracio.Tests {
    public class PrecisionTests {
        [Fact]
        public void BasicAddition_IsExact() {
            // 1/2 + 1/3 = 5/6
            using var a = new Ratio(1, 2);
            using var b = new Ratio(1, 3);
            a.Add(b);
            Assert.Equal("5/6", a.ToString());
        }

        [Fact]
        public void BasicSubtraction_IsExact() {
            // 1/2 - 1/3 = 1/6
            using var a = new Ratio(1, 2);
            using var b = new Ratio(1, 3);
            a.Subtract(b);
            Assert.Equal("1/6", a.ToString());
        }

        [Fact]
        public void BasicMultiplication_IsExact() {
            // 2/3 * 3/4 = 6/12 = 1/2
            using var a = new Ratio(2, 3);
            using var b = new Ratio(3, 4);
            a.Multiply(b);
            Assert.Equal("1/2", a.ToString());
        }

        [Fact]
        public void BasicDivision_IsExact() {
            // (1/2) / (1/4) = 2/1
            using var a = new Ratio(1, 2);
            using var b = new Ratio(1, 4);
            a.Divide(b);
            Assert.Equal("2/1", a.ToString());
        }

        [Fact]
        public void FloatTrap_IsEliminated() {
            // In IEEE 754: 0.1 + 0.2 != 0.3
            // In Gracio: 1/10 + 2/10 = 3/10
            using var a = new Ratio(0.1);
            using var b = new Ratio(0.2);
            a.Add(b);
            Assert.Equal("3/10", a.ToString());
        }

        [Fact]
        public void NegativeNumbers_AreHandled() {
            // -1/2 + 1/4 = -1/4
            using var a = new Ratio(-1, 2);
            using var b = new Ratio(1, 4);
            a.Add(b);
            Assert.Equal("-1/4", a.ToString());
        }

        [Fact]
        public void OperationChain_MaintainsPrecision() {
            // (1/3 + 1/6) * 2 = (1/2) * 2 = 1/1
            using var a = new Ratio(1, 3);
            using var b = new Ratio(1, 6);
            using var c = new Ratio(2, 1);
            
            a.Add(b).Multiply(c);
            Assert.Equal("1/1", a.ToString());
        }

        [Fact]
        public void DivisionByZero_ThrowsException() {
            using var a = new Ratio(1, 2);
            using var b = new Ratio(0, 1);
            Assert.Throws<DivideByZeroException>(() => a.Divide(b));
        }
    }
}
