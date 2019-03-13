""" 
Unit test for Matlab wrap program
Author: Matthew Sklar
Date: March 2019
"""
import os
import sys
import unittest
import filecmp

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

import template_instantiator as instantiator
import interface_parser as parser
from matlab_wrapper import MatlabWrapper


TEST_DIR = "wrap/tests/"

MATLAB_TEST_DIR = TEST_DIR + "expected-matlab/"
MATLAB_ACTUAL_DIR = TEST_DIR + "actual-matlab/"


class TestWrap(unittest.TestCase):
    def test_geometry_matlab(self):
        """ Check generation of matlab geometry wrapper """
        with open(TEST_DIR + 'geometry.h', 'r') as f:
            content = f.read()

        if not os.path.exists(MATLAB_ACTUAL_DIR):
            os.mkdir(MATLAB_ACTUAL_DIR)

        module = parser.Module.parseString(content)

        instantiator.instantiate_namespace_inplace(module)

        wrapper = MatlabWrapper(
            module=module,
            module_name='geometry',
            top_module_namespace=[''],
            ignore_classes=['']
        )

        cc_content = wrapper.wrap()
        for c in cc_content:
            if type(c) == list:
                # TODO: Create folder with namespace then create files in namespace
                pass
            else:
                with open(MATLAB_ACTUAL_DIR + c[0], 'w') as f:
                    f.write(c[1])

        self.assertTrue(filecmp.cmp(
            MATLAB_ACTUAL_DIR + 'aGlobalFunction.m', MATLAB_TEST_DIR + 'aGlobalFunction.m'))
        self.assertTrue(filecmp.cmp(MATLAB_ACTUAL_DIR + 'overloadedGlobalFunction.m',
                                    MATLAB_TEST_DIR + 'overloadedGlobalFunction.m'))
        self.assertTrue(filecmp.cmp(MATLAB_ACTUAL_DIR +
                                    'MyBase.m', MATLAB_TEST_DIR + 'MyBase.m'))
        self.assertTrue(filecmp.cmp(
            MATLAB_ACTUAL_DIR + 'MyFactorPosePoint2.m', MATLAB_TEST_DIR + 'MyFactorPose2.m'))
        self.assertTrue(filecmp.cmp(
            MATLAB_ACTUAL_DIR + 'MyTemplateMatrix.m', MATLAB_TEST_DIR + 'MyTemplateMatrix.m'))
        self.assertTrue(filecmp.cmp(
            MATLAB_ACTUAL_DIR + 'MyTemplatePoint2.m', MATLAB_TEST_DIR + 'MyTemplatePoint2.m'))
        self.assertTrue(filecmp.cmp(MATLAB_ACTUAL_DIR +
                                    'Test.m', MATLAB_TEST_DIR + 'Test.m'))

if __name__ == '__main__':
    unittest.main()